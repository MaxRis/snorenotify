/*
    SnoreNotify is a Notification Framework based on Qt
    Copyright (C) 2013-2015  Hannah von Reth <vonreth@kde.org>

    SnoreNotify is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SnoreNotify is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with SnoreNotify.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "snarlnetwork.h"
#include "libsnore/snore.h"
#include "libsnore/notification/notification_p.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QPointer>

#include <iostream>
using namespace Snore;

SnarlNetworkFrontend::SnarlNetworkFrontend():
    parser(new Parser(this))
{
    connect(this, &SnarlNetworkFrontend::enabledChanged, [this](bool enabled) {
        if (enabled) {
            tcpServer = new QTcpServer(this);
            if (!tcpServer->listen(QHostAddress::Any, port)) {
                setErrorString(tr("The port is already used by a different application."));
                return;
            }
            connect(tcpServer, &QTcpServer::newConnection, this, &SnarlNetworkFrontend::handleConnection);
        } else {
            tcpServer->deleteLater();
        }
    });
}

SnarlNetworkFrontend::~SnarlNetworkFrontend()
{
    delete parser;
}

void SnarlNetworkFrontend::slotActionInvoked(Snore::Notification notification)
{
    if (notification.isActiveIn(this)) {
        qCDebug(SNORE) << notification.closeReason();
        callback(notification, QStringLiteral("SNP/1.1/304/Notification acknowledged/"));
    }
}

void SnarlNetworkFrontend::slotNotificationClosed(Snore::Notification notification)
{

    if (notification.removeActiveIn(this)) {
        switch (notification.closeReason()) {
        case Notification::TimedOut:
            callback(notification, QStringLiteral("SNP/1.1/303/Notification timed out/"));
            break;
        case Notification::Activated:
            callback(notification, QStringLiteral("SNP/1.1/307/Notification closed/"));
            break;
        case Notification::Dismissed:
            callback(notification, QStringLiteral("SNP/1.1/302/Notification cancelled/"));
            break;
        default:
            qCWarning(SNORE) << "Unhandled close reason" << notification.closeReason();
        }
    }
}

void SnarlNetworkFrontend::handleConnection()
{
    QTcpSocket *client = tcpServer->nextPendingConnection();
    connect(client, &QTcpSocket::readyRead, this, &SnarlNetworkFrontend::handleMessages);
    connect(client, &QTcpSocket::disconnected, client, &QTcpSocket::deleteLater);
}

void SnarlNetworkFrontend::handleMessages()
{
    const QString out(QStringLiteral("SNP/1.1/0/OK"));
    QTcpSocket *client = qobject_cast<QTcpSocket *>(sender());

    QStringList messages(QString::fromLatin1(client->readAll()).trimmed().split(QStringLiteral("\r\n")));
    foreach(const QString & s, messages) {
        if (s.isEmpty()) {
            continue;
        }
        Notification noti;
        parser->parse(noti, s, client);
        if (noti.isValid()) {
            noti.addActiveIn(this);
            SnoreCore::instance().broadcastNotification(noti);
            write(client, out + QLatin1Char('/') + QString::number(noti.id()) + QLatin1String("\r\n"));
        } else {
            write(client, out + QLatin1String("\r\n"));
        }
    }
}

void SnarlNetworkFrontend::callback(Notification &sn, const QString &msg)
{
    if (sn.hints().containsPrivateValue(this, "clientSocket")) {
        QTcpSocket *client = sn.hints().privateValue(this, "clientSocket").value<QPointer<QTcpSocket>>();
        if (client) {
            write(client, msg + QString::number(sn.id()) + QLatin1String("\r\n"));
        }
    }
}

