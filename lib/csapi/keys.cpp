/******************************************************************************
 * THIS FILE IS GENERATED - ANY EDITS WILL BE OVERWRITTEN
 */

#include "keys.h"

using namespace Quotient;

UploadKeysJob::UploadKeysJob(const Omittable<DeviceKeys>& deviceKeys,
                             const OneTimeKeys& oneTimeKeys,
                             const OneTimeKeys& fallbackKeys)
    : BaseJob(HttpVerb::Post, QStringLiteral("UploadKeysJob"),
              makePath("/_matrix/client/v3", "/keys/upload"))
{
    QJsonObject _data;
    addParam<IfNotEmpty>(_data, QStringLiteral("device_keys"), deviceKeys);
    addParam<IfNotEmpty>(_data, QStringLiteral("one_time_keys"), oneTimeKeys);
    addParam<IfNotEmpty>(_data, QStringLiteral("fallback_keys"), fallbackKeys);
    setRequestData(std::move(_data));
    addExpectedKey("one_time_key_counts");
}

QueryKeysJob::QueryKeysJob(const QHash<QString, QStringList>& deviceKeys,
                           Omittable<int> timeout, const QString& token)
    : BaseJob(HttpVerb::Post, QStringLiteral("QueryKeysJob"),
              makePath("/_matrix/client/v3", "/keys/query"))
{
    QJsonObject _data;
    addParam<IfNotEmpty>(_data, QStringLiteral("timeout"), timeout);
    addParam<>(_data, QStringLiteral("device_keys"), deviceKeys);
    addParam<IfNotEmpty>(_data, QStringLiteral("token"), token);
    setRequestData(std::move(_data));
}

ClaimKeysJob::ClaimKeysJob(
    const QHash<QString, QHash<QString, QString>>& oneTimeKeys,
    Omittable<int> timeout)
    : BaseJob(HttpVerb::Post, QStringLiteral("ClaimKeysJob"),
              makePath("/_matrix/client/v3", "/keys/claim"))
{
    QJsonObject _data;
    addParam<IfNotEmpty>(_data, QStringLiteral("timeout"), timeout);
    addParam<>(_data, QStringLiteral("one_time_keys"), oneTimeKeys);
    setRequestData(std::move(_data));
    addExpectedKey("one_time_keys");
}

auto queryToGetKeysChanges(const QString& from, const QString& to)
{
    QUrlQuery _q;
    addParam<>(_q, QStringLiteral("from"), from);
    addParam<>(_q, QStringLiteral("to"), to);
    return _q;
}

QUrl GetKeysChangesJob::makeRequestUrl(QUrl baseUrl, const QString& from,
                                       const QString& to)
{
    return BaseJob::makeRequestUrl(std::move(baseUrl),
                                   makePath("/_matrix/client/v3",
                                            "/keys/changes"),
                                   queryToGetKeysChanges(from, to));
}

GetKeysChangesJob::GetKeysChangesJob(const QString& from, const QString& to)
    : BaseJob(HttpVerb::Get, QStringLiteral("GetKeysChangesJob"),
              makePath("/_matrix/client/v3", "/keys/changes"),
              queryToGetKeysChanges(from, to))
{}
