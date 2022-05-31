/******************************************************************************
 * THIS FILE IS GENERATED - ANY EDITS WILL BE OVERWRITTEN
 */

#include "knocking.h"

using namespace Quotient;

auto queryToKnockRoom(const QStringList& serverName)
{
    QUrlQuery _q;
    addParam<IfNotEmpty>(_q, QStringLiteral("server_name"), serverName);
    return _q;
}

KnockRoomJob::KnockRoomJob(const QString& roomIdOrAlias,
                           const QStringList& serverName, const QString& reason)
    : BaseJob(HttpVerb::Post, QStringLiteral("KnockRoomJob"),
              makePath("/_matrix/client/v3", "/knock/", roomIdOrAlias),
              queryToKnockRoom(serverName))
{
    QJsonObject _data;
    addParam<IfNotEmpty>(_data, QStringLiteral("reason"), reason);
    setRequestData(std::move(_data));
    addExpectedKey("room_id");
}
