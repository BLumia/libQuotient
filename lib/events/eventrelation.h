// SPDX-FileCopyrightText: 2022 Kitsune Ral <kitsune-ral@users.sf.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "converters.h"

namespace Quotient {

[[maybe_unused]] constexpr auto RelatesToKey = "m.relates_to"_ls;
[[maybe_unused]] constexpr auto RelTypeKey = "rel_type"_ls;

struct QUOTIENT_API EventRelation {
    using reltypeid_t = QLatin1String;

    QString type;
    QString eventId;
    QString key = {}; // Only used for m.annotation for now

    static constexpr auto ReplyType = "m.in_reply_to"_ls;
    static constexpr auto AnnotationType = "m.annotation"_ls;
    static constexpr auto ReplacementType = "m.replace"_ls;

    static EventRelation replyTo(QString eventId)
    {
        return { ReplyType, std::move(eventId) };
    }
    static EventRelation annotate(QString eventId, QString key)
    {
        return { AnnotationType, std::move(eventId), std::move(key) };
    }
    static EventRelation replace(QString eventId)
    {
        return { ReplacementType, std::move(eventId) };
    }

    [[deprecated("Use ReplyType variable instead")]]
    static constexpr auto Reply() { return ReplyType; }
    [[deprecated("Use AnnotationType variable instead")]] //
    static constexpr auto Annotation() { return AnnotationType; }
    [[deprecated("Use ReplacementType variable instead")]] //
    static constexpr auto Replacement() { return ReplacementType; }
};

template <>
struct QUOTIENT_API JsonObjectConverter<EventRelation> {
    static void dumpTo(QJsonObject& jo, const EventRelation& pod);
    static void fillFrom(const QJsonObject& jo, EventRelation& pod);
};

} // namespace Quotient
