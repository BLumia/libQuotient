// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "eventcontent.h"
#include "stateevent.h"

namespace Quotient {
class QUOTIENT_API PowerLevelsEventContent : public EventContent::Base {
public:
    struct Notifications {
        int room;
    };

    explicit PowerLevelsEventContent(const QJsonObject& json);

    int invite;
    int kick;
    int ban;

    int redact;

    QHash<QString, int> events;
    int eventsDefault;
    int stateDefault;

    QHash<QString, int> users;
    int usersDefault;

    Notifications notifications;

protected:
    void fillJson(QJsonObject& o) const override;
};

class QUOTIENT_API RoomPowerLevelsEvent
    : public StateEvent<PowerLevelsEventContent> {
public:
    DEFINE_EVENT_TYPEID("m.room.power_levels", RoomPowerLevelsEvent)

    explicit RoomPowerLevelsEvent(PowerLevelsEventContent&& content)
        : StateEvent(typeId(), matrixTypeId(), QString(), std::move(content))
    {}
    explicit RoomPowerLevelsEvent(const QJsonObject& obj)
        : StateEvent(typeId(), obj)
    {}

    int invite() const { return content().invite; }
    int kick() const { return content().kick; }
    int ban() const { return content().ban; }

    int redact() const { return content().redact; }

    QHash<QString, int> events() const { return content().events; }
    int eventsDefault() const { return content().eventsDefault; }
    int stateDefault() const { return content().stateDefault; }

    QHash<QString, int> users() const { return content().users; }
    int usersDefault() const { return content().usersDefault; }

    int roomNotification() const { return content().notifications.room; }

    int powerLevelForEvent(const QString& eventId) const;
    int powerLevelForState(const QString& eventId) const;
    int powerLevelForUser(const QString& userId) const;
};
REGISTER_EVENT_TYPE(RoomPowerLevelsEvent)
} // namespace Quotient
