/*
    Copyright (C) 2017 Aseman Team
    http://aseman.co

    TelegramStats is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    TelegramStats is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ASEMANCLIENTSOCKET_H
#define ASEMANCLIENTSOCKET_H

#include <QObject>
#include <QDateTime>
#include <QStringList>
#include <QTcpSocket>
#include <QVariant>
#include <QUrl>

#ifdef QT_QML_LIB
#include <QtQml>
#endif //QT_QML_LIB

#include <functional>

#include "asemanclientlib_global.h"
#include "asemanabstractclientsocket.h"

class AsemanClientSocketPrivate;
class ASEMANCLIENTLIBSHARED_EXPORT AsemanClientSocket : public AsemanAbstractClientSocket
{
    Q_OBJECT
    Q_PROPERTY(QString trustKey READ trustKey WRITE setTrustKey NOTIFY trustKeyChanged)
    Q_PROPERTY(int streamVersion READ streamVersion WRITE setStreamVersion NOTIFY streamVersionChanged)

    friend class AsemanClientSocketController;

public:
    class RequestItem;

    enum SocketErrors {
        SocketTimeOutError = 0x143665d
    };

    AsemanClientSocket(QObject *parent = Q_NULLPTR);
    virtual ~AsemanClientSocket();

    void setStreamVersion(int version);
    int streamVersion() const;

    QString trustKey() const;
    void setTrustKey(const QString &trustKey);

    bool connecting() const;

Q_SIGNALS:
    void queueFinished();
    void trusted();
    void trustError();
    void callsRejected();
    void hardBlocked();
    void trustKeyChanged();
    void streamVersionChanged();

public Q_SLOTS:
    void startDestroying();
    void wake();
    void sleep();

    qint64 ping(const QString &service, int version, qint64 id, int priority) {
        return pushRequest(service, version, QStringLiteral("ping"), QVariantList()<<id, priority, true);
    }
    qint64 pushRequest(const QString &service, int version, const QString &method, const QVariantList &args, int priority, bool hasResult);

protected Q_SLOTS:
    void initSocket();

private Q_SLOTS:
    void onReadyRead();
    void writeQueue();
    void goTrust();
    void connectedSlt();
    void disconnected();

private:
    bool write(qint64 id, bool queueFailed = true);
    QByteArray read(qint64 maxlen = 0);

    void startTimeOut();
    void checkTimeOut();
    void timedOut();

    void resolveTrust(QByteArray data);

protected:
    void trustUsingKey(const QString &key = QString());
    void hardPing(qint64 id);
    void handleError(qint64 error, QDataStream &stream);
    void trustResolved(const QByteArray &data);

private:
    AsemanClientSocketPrivate *p;
};

#endif // ASEMANCLIENTSOCKET_H
