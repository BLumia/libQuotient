// SPDX-FileCopyrightText: 2018 Kitsune Ral <Kitsune-Ral@users.sf.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "event.h"

#include <QtCore/QDateTime>
#include <QtCore/QVector>

namespace Quotient {
struct UserTimestamp {
    QString userId;
    QDateTime timestamp;
};
struct ReceiptsForEvent {
    QString evtId;
    QVector<UserTimestamp> receipts;
};
using EventsWithReceipts = QVector<ReceiptsForEvent>;

class QUOTIENT_API ReceiptEvent : public Event {
public:
    DEFINE_EVENT_TYPEID("m.receipt", ReceiptEvent)
    explicit ReceiptEvent(const EventsWithReceipts& ewrs);
    explicit ReceiptEvent(const QJsonObject& obj) : Event(typeId(), obj) {}

    EventsWithReceipts eventsWithReceipts() const;
};
REGISTER_EVENT_TYPE(ReceiptEvent)
} // namespace Quotient
