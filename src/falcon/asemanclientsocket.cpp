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

#include "asemanclientsocket.h"
#include "asemantrustsolverengine.h"
#include "../AsemanGlobals/asemanserverenums.h"
#include "asemanclientsocketcontroller.h"

#define DISABLE_FLUSH
#define CLASS_NAME QString(QString::fromUtf8(metaObject()->className()) + ":").toStdString().c_str()

#include <QTcpSocket>
#include <QDataStream>
#include <QPointer>
#include <QTimerEvent>
#include <QQueue>
#include <QTimer>
#include <QBuffer>
#include <QThread>
#include <QTcpSocket>
#include <QFile>

class AsemanClientSocket::RequestItem
{
public:
    qint64 id = 0;
    qint32 priority = 1000;
    QByteArray data;
    QDateTime writtenDate;
    QDateTime addedDate = QDateTime::currentDateTime();
    bool hasResult = true;

    QDateTime timeout() const {
        return addedDate.addSecs(30);
    }
};

class AsemanClientSocketPrivate
{
public:
    int streamVersion;
    qint64 id_counter;
    AsemanClientSocketController *controller;

    QMap<qint64, AsemanClientSocket::RequestItem> requests;

    QTimer *destroyTimer;
    QTimer *request_timeoutTimer;

    AsemanTrustSolverEngine *solver;

    QByteArray buffer;
    QString trustKey;
    bool trusted;
};

AsemanClientSocket::AsemanClientSocket(QObject *parent) :
    AsemanAbstractClientSocket(parent)
{
    p = new AsemanClientSocketPrivate;
    p->streamVersion = QDataStream::Qt_5_6;
    p->solver = Q_NULLPTR;
    p->trusted = false;
    p->id_counter = 10000;

    p->controller = new AsemanClientSocketController(this);

    p->destroyTimer = new QTimer(this);
    p->destroyTimer->setInterval(2000);
    p->destroyTimer->setSingleShot(true);

    p->request_timeoutTimer = new QTimer(this);
    p->request_timeoutTimer->setSingleShot(true);

    initSocket();
    setHostAddress(QStringLiteral("aseman.center"));
    setPort(4118);

    connect(p->destroyTimer, &QTimer::timeout, this, &AsemanClientSocket::deleteLater);
    connect(p->request_timeoutTimer, &QTimer::timeout, this, &AsemanClientSocket::checkTimeOut);
    connect(this, &AsemanClientSocket::trusted, this, &AsemanClientSocket::connectedSlt, Qt::QueuedConnection);

    connect(p->controller, &AsemanClientSocketController::connected, this, &AsemanClientSocket::goTrust);
    connect(p->controller, &AsemanClientSocketController::disconnected, this, &AsemanClientSocket::disconnected);
    connect(p->controller, &AsemanClientSocketController::generalError, this, &AsemanClientSocket::generalError);
    connect(p->controller, &AsemanClientSocketController::readyRead, this, &AsemanClientSocket::onReadyRead);
    connect(p->controller, &AsemanClientSocketController::connectingChanged, this, &AsemanClientSocket::connectingChanged);
}

void AsemanClientSocket::setStreamVersion(int streamVersion)
{
    if(p->streamVersion == streamVersion)
        return;

    p->streamVersion = streamVersion;
    Q_EMIT streamVersionChanged();
}

int AsemanClientSocket::streamVersion() const
{
    return p->streamVersion;
}

qint64 AsemanClientSocket::pushRequest(const QString &service, int version, const QString &method, const QVariantList &args, int priority, bool hasResult)
{
    p->id_counter++;

    QByteArray structData;
    QDataStream structStream(&structData, QIODevice::WriteOnly);
    structStream.setVersion(streamVersion());
    structStream << method;
    structStream << args;

    QByteArray topData;
    QDataStream topStream(&topData, QIODevice::WriteOnly);
    topStream.setVersion(QDataStream::Qt_5_6);
    topStream << service;
    topStream << version;
    topStream << p->id_counter;
    topStream << structData;

    QByteArray hardData;
    QDataStream hardStream(&hardData, QIODevice::WriteOnly);
    hardStream.setVersion(QDataStream::Qt_5_6);
    hardStream << static_cast<qint64>(AsemanServerEnums::HardOprServiceCall);
    hardStream << topData;

    AsemanClientSocket::RequestItem req;
    req.id = p->id_counter;
    req.data = hardData;
    req.priority = priority;
    req.hasResult = hasResult;

    p->requests[req.id] = req;
    if(p->trusted || !trustingSystem())
        write(req.id);

    startTimeOut();
    return p->id_counter;
}

QString AsemanClientSocket::trustKey() const
{
    return p->trustKey;
}

void AsemanClientSocket::setTrustKey(const QString &trustKey)
{
    if(p->trustKey == trustKey)
        return;

    p->trustKey = trustKey;
    Q_EMIT trustKeyChanged();
}

bool AsemanClientSocket::connecting() const
{
    return p->controller->connecting();
}

void AsemanClientSocket::trustUsingKey(const QString &key)
{
    QByteArray structData;
    if(!key.isEmpty())
    {
        QDataStream topStream(&structData, QIODevice::WriteOnly);
        topStream.setVersion(QDataStream::Qt_5_6);
        topStream << key;
    }

    QByteArray hardData;
    QDataStream hardStream(&hardData, QIODevice::WriteOnly);
    hardStream.setVersion(QDataStream::Qt_5_6);
    if(key.isEmpty())
        hardStream << static_cast<qint64>(AsemanServerEnums::HardOprTrustRequest);
    else
        hardStream << static_cast<qint64>(AsemanServerEnums::HardOprTrustUsingKey);
    hardStream << structData;

    p->controller->write(hardData);
    setTrustKey(key);
}

void AsemanClientSocket::hardPing(qint64 id)
{
    QByteArray structData;
    QDataStream topStream(&structData, QIODevice::WriteOnly);
    topStream.setVersion(QDataStream::Qt_5_6);
    topStream << id;

    QByteArray hardData;
    QDataStream hardStream(&hardData, QIODevice::WriteOnly);
    hardStream.setVersion(QDataStream::Qt_5_6);
    hardStream << static_cast<qint64>(AsemanServerEnums::HardOprHardPing);
    hardStream << structData;

    p->controller->write(hardData);
}

void AsemanClientSocket::startDestroying()
{
    p->destroyTimer->stop();
    if(p->requests.isEmpty())
        p->destroyTimer->start();
}

void AsemanClientSocket::wake()
{
    p->controller->wake();
}

void AsemanClientSocket::sleep()
{
    p->controller->sleep();
}

void AsemanClientSocket::onReadyRead()
{
    while(!p->controller->atEnd() || !p->buffer.isEmpty())
    {
        QByteArray data = read();

        QBuffer buffer(&data);
        buffer.open(QBuffer::ReadOnly);

        QDataStream hardStream(&buffer);
        hardStream.setVersion(QDataStream::Qt_5_6);

        qint64 hardOperator = 0;
        QByteArray hardData;

        hardStream >> hardOperator;
        hardStream >> hardData;

        if(hardStream.status() == QDataStream::ReadPastEnd)
            break;

        QDataStream stream(&hardData, QIODevice::ReadOnly);
        switch(hardOperator)
        {
        case AsemanServerEnums::HardOprTrustRequest:
        {
            resolveTrust(hardData);
            qDebug() << CLASS_NAME << QStringLiteral("Trusting...");
        }
            break;

        case AsemanServerEnums::HardOprTrustResult:
        {
            QString trustKey;
            stream >> trustKey;
            if(trustKey.isEmpty())
            {
                qDebug() << CLASS_NAME << QStringLiteral("Trust question not solved :(");
                Q_EMIT trustError();
            }
            else
            {
                setTrustKey(trustKey);
                qDebug() << CLASS_NAME << QStringLiteral("Trusted") << trustKey;
                p->trusted = true;
                Q_EMIT trusted();
            }
        }
            break;

        case AsemanServerEnums::HardOprHardPing:
        {
            qint64 pingId;
            stream >> pingId;
            Q_UNUSED(pingId)
        }
            break;

        case AsemanServerEnums::HardOprTrustUsingKey:
        {
            bool result;
            stream >> result;
            p->trusted = result;
            if(result) {
                qDebug() << CLASS_NAME << QStringLiteral("Trusted using key.");
                Q_EMIT trusted();
            } else {
                qDebug() << CLASS_NAME << QStringLiteral("Trust key expired:") << p->trustKey;
                p->trusted = false;
                if(autoTrust())
                    trustUsingKey();
                Q_EMIT trustError();
            }
        }
            break;

        case AsemanServerEnums::HardOprError:
        {
            qint64 error;
            stream >> error;
            handleError(error, stream);
        }
            break;

        case AsemanServerEnums::HardOprServiceCall:
        {
            qint64 uniqueId = 0;
            stream >> uniqueId;

            int removed = p->requests.remove(uniqueId);

            QByteArray serviceData;
            stream >> serviceData;

            if(serviceData.isEmpty())
            {
                if(stream.status() != QDataStream::ReadPastEnd)
                    p->buffer = p->buffer.mid(static_cast<qint32>(buffer.pos()));
                continue;
            }

            QVariant result;

            QDataStream resultStream(&serviceData, QIODevice::ReadOnly);
            resultStream.setVersion(streamVersion());
            resultStream >> result;

            if(removed != 0)
                Q_EMIT answer(uniqueId, result);
        }
            break;

        case AsemanServerEnums::HardOprServiceSignal:
        {
            QString signalName;
            QVariantList args;

            stream >> signalName;
            stream >> args;

            Q_EMIT signalEmitted(signalName, args);
        }
            break;

        default:
            qDebug() << CLASS_NAME << QStringLiteral("Bad result :|");
            break;
        }

        p->buffer = p->buffer.mid(static_cast<qint32>(buffer.pos()));
    }
}
bool writeQueue_priority_LessThan(const AsemanClientSocket::RequestItem &r1, const AsemanClientSocket::RequestItem &r2)
  {
      return r1.priority > r2.priority;
  }

void AsemanClientSocket::writeQueue()
{
    qDebug() << CLASS_NAME << QStringLiteral("Api connected :)");
    if(!p->trusted && autoTrust())
        return;

    QList<AsemanClientSocket::RequestItem> requests = p->requests.values();
    std::sort(requests.begin(), requests.end(), writeQueue_priority_LessThan);

    bool breaked = false;
    for(const AsemanClientSocket::RequestItem &req: requests)
    {
        bool ok = write(req.id, false);
        if(!ok)
        {
            breaked = true;
            break;
        }
    }

    if(breaked && p->controller->isConnected())
    {
        qDebug() << CLASS_NAME << QStringLiteral("Failed to write all of the queue. Retrying...");
        QTimer::singleShot(5000, this, &AsemanClientSocket::writeQueue);
    }
}

void AsemanClientSocket::goTrust()
{
    if(autoTrust())
        trustUsingKey(p->trustKey);
    else
        connectedSlt();
}

void AsemanClientSocket::connectedSlt()
{
    writeQueue();
    Q_EMIT connected();
}

void AsemanClientSocket::disconnected()
{
    qDebug() << CLASS_NAME << QStringLiteral("Disconnected :(");
}

bool AsemanClientSocket::write(qint64 id, bool queueFailed)
{
    if(!p->requests.contains(id))
        return false;

    AsemanClientSocket::RequestItem req = p->requests.take(id);
    p->destroyTimer->stop();

    bool result = false;
    if(p->controller->isConnected())
    {
        p->controller->write(req.data);
#ifdef DISABLE_FLUSH
        result = true;
#else
        result = p->socket->flush();
#endif
        if(result)
            req.writtenDate = QDateTime::currentDateTime();
    }
    if(queueFailed && !result)
        req.writtenDate = QDateTime();

    if(req.hasResult || !result)
        p->requests[id] = req;

    return result;
}

QByteArray AsemanClientSocket::read(qint64 maxlen)
{
    p->buffer += maxlen? p->controller->read(maxlen) : p->controller->readAll();
    return p->buffer;
}

void AsemanClientSocket::startTimeOut()
{
    p->request_timeoutTimer->stop();

    QDateTime currentDate = QDateTime::currentDateTime();
    QDateTime minimumDate;

    QMapIterator<qint64, AsemanClientSocket::RequestItem> i(p->requests);
    while(i.hasNext())
    {
        i.next();
        AsemanClientSocket::RequestItem &res = p->requests[i.key()];
        if(res.timeout() < currentDate)
            continue;
        if(!minimumDate.isNull() && res.timeout() >= minimumDate)
            continue;

        minimumDate = res.timeout();
    }

    if(minimumDate.isNull())
        return;

    qint32 ms = static_cast<qint32>(currentDate.msecsTo(minimumDate));

    p->request_timeoutTimer->setInterval(ms);
    p->request_timeoutTimer->start();
}

void AsemanClientSocket::checkTimeOut()
{
    QDateTime currentDate = QDateTime::currentDateTime();

    QMapIterator<qint64, AsemanClientSocket::RequestItem> i(p->requests);
    while(i.hasNext())
    {
        i.next();
        const AsemanClientSocket::RequestItem res = p->requests.value(i.key());
        if(res.timeout() >= currentDate)
            continue;

        p->requests.remove(res.id);

        CallbackError errorValue;
        errorValue.null = false;
        errorValue.errorCode = SocketTimeOutError;
        errorValue.errorValue = QStringLiteral("Timed out");

        Q_EMIT error(res.id, errorValue);
        Q_EMIT error(res.id, SocketTimeOutError, QStringLiteral("Timed out"));
    }

    startTimeOut();
}

void AsemanClientSocket::timedOut()
{
    sleep();
    wake();
}

void AsemanClientSocket::initSocket()
{
    p->controller->reconnect();
}

void AsemanClientSocket::resolveTrust(QByteArray data)
{
    if(!p->solver)
    {
        QThread *thread = new QThread(this);
        thread->start();

        p->solver = new AsemanTrustSolverEngine();
        p->solver->moveToThread(thread);

        connect(p->solver, &AsemanTrustSolverEngine::solved, this, &AsemanClientSocket::trustResolved);
    }

    QMetaObject::invokeMethod(p->solver, "resolve", Qt::QueuedConnection, Q_ARG(QByteArray, data));
}

void AsemanClientSocket::handleError(qint64 error, QDataStream &stream)
{
    switch(error)
    {
    case AsemanServerEnums::HardOprErrTrustNeeded:
        p->trusted = false;
        trustUsingKey();
        break;

    case AsemanServerEnums::HardOprErrBadCommand:
        break;

    case AsemanServerEnums::HardOprErrBadStructure:
        break;

    case AsemanServerEnums::HardOprErrRejectAllCalls:
        p->requests.clear();
        p->request_timeoutTimer->stop();
        Q_EMIT callsRejected();
        break;

    case AsemanServerEnums::HardOprErrHostBlocked:
        Q_EMIT hardBlocked();
        break;

    case AsemanServerEnums::HardOprErrBadServiceRequest:
        break;

    case AsemanServerEnums::HardOprErrDynamicError:
    {
        QByteArray data;
        qint64 uniqueId = 0;
        stream >> uniqueId;
        stream >> data;

        QVariant variant;
        QDataStream errorStream(&data, QIODevice::ReadOnly);
        errorStream.setVersion(streamVersion());
        errorStream >> variant;

        CallbackError errorValue;
        errorValue.null = false;
        if(variant.type() == QVariant::Map)
        {
            QVariantMap map = variant.toMap();
            errorValue.errorCode = map.value(QStringLiteral("code")).toInt();
            errorValue.errorValue = map.value(QStringLiteral("value"));
        }
        else
            errorValue.errorValue = variant;

        Q_EMIT AsemanAbstractClientSocket::error(uniqueId, errorValue);
        Q_EMIT AsemanAbstractClientSocket::error(uniqueId, errorValue.errorCode, errorValue.errorValue);
    }
        break;
    }
}

void AsemanClientSocket::trustResolved(const QByteArray &data)
{
    QByteArray hardData;
    QDataStream hardStream(&hardData, QIODevice::WriteOnly);
    hardStream.setVersion(QDataStream::Qt_5_6);
    hardStream << static_cast<qint64>(AsemanServerEnums::HardOprTrustResult);
    hardStream << data;

    p->controller->write(hardData);
}

AsemanClientSocket::~AsemanClientSocket()
{
    if(p->controller->isConnected())
        disconnected();
    if(p->solver)
    {
        QThread *thread = p->solver->thread();
        p->solver->deleteLater();
        thread->quit();
        thread->wait();
        thread->deleteLater();
    }
    delete p;
}
