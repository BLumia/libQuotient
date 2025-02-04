// SPDX-FileCopyrightText: 2018 Kitsune Ral <kitsune-ral@users.sf.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "networkaccessmanager.h"

#include "connection.h"
#include "room.h"
#include "accountregistry.h"
#include "mxcreply.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QThread>
#include <QtCore/QSettings>
#include <QtNetwork/QNetworkReply>

using namespace Quotient;

class NetworkAccessManager::Private {
public:
    explicit Private(NetworkAccessManager* q)
        : q(q)
    {}

    QNetworkReply* createImplRequest(Operation op,
                                     const QNetworkRequest& outerRequest,
                                     Connection* connection)
    {
        Q_ASSERT(outerRequest.url().scheme() == "mxc");
        QNetworkRequest r(outerRequest);
        r.setUrl(QUrl(QStringLiteral("%1/_matrix/media/r0/download/%2")
                          .arg(connection->homeserver().toString(),
                               outerRequest.url().authority()
                                   + outerRequest.url().path())));
        return q->createRequest(op, r);
    }

    NetworkAccessManager* q;
    QList<QSslError> ignoredSslErrors;
};

NetworkAccessManager::NetworkAccessManager(QObject* parent)
    : QNetworkAccessManager(parent), d(makeImpl<Private>(this))
{}

QList<QSslError> NetworkAccessManager::ignoredSslErrors() const
{
    return d->ignoredSslErrors;
}

void NetworkAccessManager::ignoreSslErrors(bool ignore) const
{
    if (ignore) {
        connect(this, &QNetworkAccessManager::sslErrors, this,
                [](QNetworkReply* reply) { reply->ignoreSslErrors(); });
    } else {
        disconnect(this, &QNetworkAccessManager::sslErrors, this, nullptr);
    }
}

void NetworkAccessManager::addIgnoredSslError(const QSslError& error)
{
    d->ignoredSslErrors << error;
}

void NetworkAccessManager::clearIgnoredSslErrors()
{
    d->ignoredSslErrors.clear();
}

NetworkAccessManager* NetworkAccessManager::instance()
{
    thread_local auto* nam = [] {
        auto* namInit = new NetworkAccessManager();
        connect(QThread::currentThread(), &QThread::finished, namInit,
                &QObject::deleteLater);
        return namInit;
    }();
    return nam;
}

QNetworkReply* NetworkAccessManager::createRequest(
    Operation op, const QNetworkRequest& request, QIODevice* outgoingData)
{
    const auto& mxcUrl = request.url();
    if (mxcUrl.scheme() == "mxc") {
        const QUrlQuery query(mxcUrl.query());
        const auto accountId = query.queryItemValue(QStringLiteral("user_id"));
        if (accountId.isEmpty()) {
            // Using QSettings here because Quotient::NetworkSettings
            // doesn't provide multithreading guarantees
            static thread_local QSettings s;
            if (!s.value("Network/allow_direct_media_requests").toBool()) {
                qCWarning(NETWORK) << "No connection specified";
                return new MxcReply();
            }
            // TODO: Make the best effort with a direct unauthenticated request
            // to the media server
        } else {
            auto* const connection = Accounts.get(accountId);
            if (!connection) {
                qCWarning(NETWORK) << "Connection" << accountId << "not found";
                return new MxcReply();
            }
            const auto roomId = query.queryItemValue(QStringLiteral("room_id"));
            if (!roomId.isEmpty()) {
                auto room = connection->room(roomId);
                if (!room) {
                    qCWarning(NETWORK) << "Room" << roomId << "not found";
                    return new MxcReply();
                }
                return new MxcReply(
                    d->createImplRequest(op, request, connection), room,
                    query.queryItemValue(QStringLiteral("event_id")));
            }
            return new MxcReply(
                d->createImplRequest(op, request, connection));
        }
    }
    auto reply = QNetworkAccessManager::createRequest(op, request, outgoingData);
    reply->ignoreSslErrors(d->ignoredSslErrors);
    return reply;
}

QStringList NetworkAccessManager::supportedSchemesImplementation() const
{
    auto schemes = QNetworkAccessManager::supportedSchemesImplementation();
    schemes += QStringLiteral("mxc");
    return schemes;
}
