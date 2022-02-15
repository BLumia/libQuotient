// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "e2ee/e2ee.h"
#include "e2ee/qolmerrors.h"
#include "olm/olm.h"

#include <memory>
#include <variant>

namespace Quotient {

//! An in-bound group session is responsible for decrypting incoming
//! communication in a Megolm session.
class QUOTIENT_API QOlmInboundGroupSession
{
public:
    ~QOlmInboundGroupSession();
    //! Creates a new instance of `OlmInboundGroupSession`.
    static std::unique_ptr<QOlmInboundGroupSession> create(const QByteArray& key);
    //! Import an inbound group session, from a previous export.
    static std::unique_ptr<QOlmInboundGroupSession> import(const QByteArray& key);
    //! Serialises an `OlmInboundGroupSession` to encrypted Base64.
    QByteArray pickle(const PicklingMode &mode) const;
    //! Deserialises from encrypted Base64 that was previously obtained by pickling
    //! an `OlmInboundGroupSession`.
    static std::variant<std::unique_ptr<QOlmInboundGroupSession>, QOlmError>
    unpickle(const QByteArray& picked, const PicklingMode& mode);
    //! Decrypts ciphertext received for this group session.
    std::variant<std::pair<QString, uint32_t>, QOlmError> decrypt(
        const QByteArray& message);
    //! Export the base64-encoded ratchet key for this session, at the given index,
    //! in a format which can be used by import.
    std::variant<QByteArray, QOlmError> exportSession(uint32_t messageIndex);
    //! Get the first message index we know how to decrypt.
    uint32_t firstKnownIndex() const;
    //! Get a base64-encoded identifier for this session.
    QByteArray sessionId() const;
    bool isVerified() const;

    QOlmInboundGroupSession(OlmInboundGroupSession* session);
private:
    OlmInboundGroupSession* m_groupSession;
};

using QOlmInboundGroupSessionPtr = std::unique_ptr<QOlmInboundGroupSession>;
} // namespace Quotient
