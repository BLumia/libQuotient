// SPDX-FileCopyrightText: 2016 Kitsune Ral <kitsune-ral@users.sf.net>
// SPDX-FileCopyrightText: 2019 Alexey Andreyev <aa13q@ya.ru>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "quotient_export.h"

#include <QtCore/QLatin1String>
#include <QtCore/QHashFunctions>

#include <functional>
#include <memory>
#include <unordered_map>
#include <optional>

#ifndef Q_DISABLE_MOVE
// Q_DISABLE_MOVE was introduced in Q_VERSION_CHECK(5,13,0)
#    define Q_DISABLE_MOVE(_ClassName)             \
        _ClassName(_ClassName&&) Q_DECL_EQ_DELETE; \
        _ClassName& operator=(_ClassName&&) Q_DECL_EQ_DELETE;
#endif

#ifndef Q_DISABLE_COPY_MOVE
#define Q_DISABLE_COPY_MOVE(Class) \
    Q_DISABLE_COPY(Class) \
    Q_DISABLE_MOVE(Class)
#endif

#define DISABLE_MOVE(_ClassName) \
static_assert(false, "Use Q_DISABLE_MOVE instead; Quotient enables it across all used versions of Qt");

#ifndef QT_IGNORE_DEPRECATIONS
// QT_IGNORE_DEPRECATIONS was introduced in Q_VERSION_CHECK(5,15,0)
#    define QT_IGNORE_DEPRECATIONS(statement) \
        QT_WARNING_PUSH                       \
        QT_WARNING_DISABLE_DEPRECATED         \
        statement                             \
        QT_WARNING_POP
#endif

namespace Quotient {
/// An equivalent of std::hash for QTypes to enable std::unordered_map<QType, ...>
template <typename T>
struct HashQ {
    size_t operator()(const T& s) const Q_DECL_NOEXCEPT
    {
        return qHash(s, uint(qGlobalQHashSeed()));
    }
};
/// A wrapper around std::unordered_map compatible with types that have qHash
template <typename KeyT, typename ValT>
using UnorderedMap = std::unordered_map<KeyT, ValT, HashQ<KeyT>>;

namespace _impl {
    template <typename TT>
    constexpr inline auto IsOmittableValue = false;
    template <typename TT>
    constexpr inline auto IsOmittable = IsOmittableValue<std::decay_t<TT>>;
}

constexpr auto none = std::nullopt;

/** `std::optional` with tweaks
 *
 * The tweaks are:
 * - streamlined assignment (operator=)/emplace()ment of values that can be
 *   used to implicitly construct the underlying type, including
 *   direct-list-initialisation, e.g.:
 *   \code
 *   struct S { int a; char b; }
 *   Omittable<S> o;
 *   o = { 1, 'a' }; // std::optional would require o = S { 1, 'a' }
 *   \endcode
 * - entirely deleted value(). The technical reason is that Xcode 10 doesn't
 *   have it; but besides that, value_or() or (after explicit checking)
 *   `operator*()`/`operator->()` are better alternatives within Quotient
 *   that doesn't practice throwing exceptions (as doesn't most of Qt).
 * - disabled non-const lvalue operator*() and operator->(), as it's too easy
 *   to inadvertently cause a value change through them.
 * - edit() to provide a safe and explicit lvalue accessor instead of those
 *   above. Requires the underlying type to be default-constructible.
 *   Allows chained initialisation of nested Omittables:
 *   \code
 *   struct Inner { int member = 10; Omittable<int> innermost; };
 *   struct Outer { int anotherMember = 10; Omittable<Inner> inner; };
 *   Omittable<Outer> o; // = { 10, std::nullopt };
 *   o.edit().inner.edit().innermost.emplace(42);
 *   \endcode
 * - merge() - a soft version of operator= that only overwrites its first
 *   operand with the second one if the second one is not empty.
 */
template <typename T>
class Omittable : public std::optional<T> {
public:
    using base_type = std::optional<T>;
    using value_type = std::decay_t<T>;

    using std::optional<T>::optional;

    // Overload emplace() and operator=() to allow passing braced-init-lists
    // (the standard emplace() does direct-initialisation but
    // not direct-list-initialisation).
    using base_type::operator=;
    Omittable& operator=(const value_type& v)
    {
        base_type::operator=(v);
        return *this;
    }
    Omittable& operator=(value_type&& v)
    {
        base_type::operator=(v);
        return *this;
    }
    using base_type::emplace;
    T& emplace(const T& val) { return base_type::emplace(val); }
    T& emplace(T&& val) { return base_type::emplace(std::move(val)); }

    // use value_or() or check (with operator! or has_value) before accessing
    // with operator-> or operator*
    // The technical reason is that Xcode 10 has incomplete std::optional
    // that has no value(); but using value() may also mean that you rely
    // on the optional throwing an exception (which is not assumed practice
    // throughout Quotient) or that you spend unnecessary CPU cycles on
    // an extraneous has_value() check.
    value_type& value() = delete;
    const value_type& value() const = delete;
    value_type& edit()
    {
        return this->has_value() ? base_type::operator*() : this->emplace();
    }

    [[deprecated("Use '!o' or '!o.has_value()' instead of 'o.omitted()'")]]
    bool omitted() const
    {
        return !this->has_value();
    }

    //! Merge the value from another Omittable
    //! \return true if \p other is not omitted and the value of
    //!         the current Omittable was different (or omitted),
    //!         in other words, if the current Omittable has changed;
    //!         false otherwise
    template <typename T1>
    auto merge(const Omittable<T1>& other)
        -> std::enable_if_t<std::is_convertible_v<T1, T>, bool>
    {
        if (!other || (this->has_value() && **this == *other))
            return false;
        emplace(*other);
        return true;
    }

    // Hide non-const lvalue operator-> and operator* as these are
    // a bit too surprising: value() & doesn't lazy-create an object;
    // and it's too easy to inadvertently change the underlying value.

    const value_type* operator->() const& { return base_type::operator->(); }
    value_type* operator->() && { return base_type::operator->(); }
    const value_type& operator*() const& { return base_type::operator*(); }
    value_type& operator*() && { return base_type::operator*(); }
};
template <typename T>
Omittable(T&&) -> Omittable<T>;

namespace _impl {
    template <typename T>
    constexpr inline auto IsOmittableValue<Omittable<T>> = true;
}

template <typename T1, typename T2>
inline auto merge(Omittable<T1>& lhs, T2&& rhs)
{
    return lhs.merge(std::forward<T2>(rhs));
}

//! \brief Merge the value from an Omittable
//! This is an adaptation of Omittable::merge() to the case when the value
//! on the left hand side is not an Omittable.
//! \return true if \p rhs is not omitted and the \p lhs value was different,
//!         in other words, if \p lhs has changed;
//!         false otherwise
template <typename T1, typename T2>
inline auto merge(T1& lhs, const Omittable<T2>& rhs)
    -> std::enable_if_t<!_impl::IsOmittable<T1>
                            && std::is_convertible_v<T2, T1>, bool>
{
    if (!rhs || lhs == *rhs)
        return false;
    lhs = *rhs;
    return true;
}

inline constexpr auto operator"" _ls(const char* s, std::size_t size)
{
    return QLatin1String(s, int(size));
}

/** An abstraction over a pair of iterators
 * This is a very basic range type over a container with iterators that
 * are at least ForwardIterators. Inspired by Ranges TS.
 */
template <typename ArrayT>
class Range {
    // Looking forward to C++20 ranges
    using iterator = typename ArrayT::iterator;
    using const_iterator = typename ArrayT::const_iterator;
    using size_type = typename ArrayT::size_type;

public:
    constexpr Range(ArrayT& arr) : from(std::begin(arr)), to(std::end(arr)) {}
    constexpr Range(iterator from, iterator to) : from(from), to(to) {}

    constexpr size_type size() const
    {
        Q_ASSERT(std::distance(from, to) >= 0);
        return size_type(std::distance(from, to));
    }
    constexpr bool empty() const { return from == to; }
    constexpr const_iterator begin() const { return from; }
    constexpr const_iterator end() const { return to; }
    constexpr iterator begin() { return from; }
    constexpr iterator end() { return to; }

private:
    iterator from;
    iterator to;
};

/** A replica of std::find_first_of that returns a pair of iterators
 *
 * Convenient for cases when you need to know which particular "first of"
 * [sFirst, sLast) has been found in [first, last).
 */
template <typename InputIt, typename ForwardIt, typename Pred>
inline std::pair<InputIt, ForwardIt> findFirstOf(InputIt first, InputIt last,
                                                    ForwardIt sFirst,
                                                    ForwardIt sLast, Pred pred)
{
    for (; first != last; ++first)
        for (auto it = sFirst; it != sLast; ++it)
            if (pred(*first, *it))
                return std::make_pair(first, it);

    return std::make_pair(last, sLast);
}

//! \brief An owning implementation pointer
//!
//! This is basically std::unique_ptr<> to hold your pimpl's but without having
//! to define default constructors/operator=() out of line.
//! Thanks to https://oliora.github.io/2015/12/29/pimpl-and-rule-of-zero.html
//! for inspiration
template <typename ImplType>
using ImplPtr = std::unique_ptr<ImplType, void (*)(ImplType*)>;

// Why this works (see also the link above): because this defers the moment
// of requiring sizeof of ImplType to the place where makeImpl is invoked
// (which is located, necessarily, in the .cpp file after ImplType definition).
// The stock unique_ptr deleter (std::default_delete) normally needs sizeof
// at the same spot - as long as you defer definition of the owning type
// constructors and operator='s to the .cpp file as well. Which means you
// have to explicitly declare and define them (even if with = default),
// formally breaking the rule of zero; informally, just adding boilerplate code.
// The custom deleter itself is instantiated at makeImpl invocation - there's
// no way earlier to even know how ImplType will be deleted and whether that
// will need sizeof(ImplType) earlier. In theory it's a tad slower because
// the deleter is called by the pointer; however, the difference will not
// be noticeable (if exist at all) for any class with non-trivial contents.

//! \brief make_unique for ImplPtr
//!
//! Since std::make_unique is not compatible with ImplPtr, this should be used
//! in constructors of frontend classes to create implementation instances.
template <typename ImplType, typename DeleterType = void (*)(ImplType*),
          typename... ArgTs>
inline ImplPtr<ImplType> makeImpl(ArgTs&&... args)
{
    return ImplPtr<ImplType> { new ImplType(std::forward<ArgTs>(args)...),
                               [](ImplType* impl) { delete impl; } };
}

template <typename ImplType>
const inline ImplPtr<ImplType> ZeroImpl()
{
    return { nullptr, [](ImplType*) {} };
}

/** Convert what looks like a URL or a Matrix ID to an HTML hyperlink */
QUOTIENT_API void linkifyUrls(QString& htmlEscapedText);

/** Sanitize the text before showing in HTML
 *
 * This does toHtmlEscaped() and removes Unicode BiDi marks.
 */
QUOTIENT_API QString sanitized(const QString& plainText);

/** Pretty-print plain text into HTML
 *
 * This includes HTML escaping of <,>,",& and calling linkifyUrls()
 */
QUOTIENT_API QString prettyPrint(const QString& plainText);

/** Return a path to cache directory after making sure that it exists
 *
 * The returned path has a trailing slash, clients don't need to append it.
 * \param dir path to cache directory relative to the standard cache path
 */
QUOTIENT_API QString cacheLocation(const QString& dirName);

/** Hue color component of based of the hash of the string.
 *
 * The implementation is based on XEP-0392:
 * https://xmpp.org/extensions/xep-0392.html
 * Naming and range are the same as QColor's hueF method:
 * https://doc.qt.io/qt-5/qcolor.html#integer-vs-floating-point-precision
 */
QUOTIENT_API qreal stringToHueF(const QString& s);

/** Extract the serverpart from MXID */
QUOTIENT_API QString serverPart(const QString& mxId);

QUOTIENT_API QString versionString();
QUOTIENT_API int majorVersion();
QUOTIENT_API int minorVersion();
QUOTIENT_API int patchVersion();
} // namespace Quotient
