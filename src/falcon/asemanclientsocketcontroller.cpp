#include "asemanclientsocketcontroller.h"
#include "asemanclientsocket.h"
#include "../AsemanGlobals/asemanserverenums.h"

#include <QPointer>
#include <QSslSocket>
#include <QTcpSocket>
#include <QTimer>
#include <QDataStream>

class AsemanClientSocketController::Private
{
public:
    QPointer<QTcpSocket> socket;
    AsemanClientSocket *parent;
    bool waked;
    bool connecting;

    QTimer *socketTimeout;
    QTimer *pingTimeout;

    qint32 pingCounter;
};

AsemanClientSocketController::AsemanClientSocketController(AsemanClientSocket *parent) :
    QObject(parent)
{
    p = new Private;
    p->parent = parent;
    p->pingCounter = 1000;
    p->waked = false;
    p->connecting = false;

    p->socketTimeout = new QTimer(this);
    p->socketTimeout->setInterval(20000);
    p->socketTimeout->setSingleShot(true);

    p->pingTimeout = new QTimer(this);
    p->pingTimeout->setInterval(10000);
    p->pingTimeout->setSingleShot(true);

    connect(p->socketTimeout, &QTimer::timeout, this, &AsemanClientSocketController::socketTimeout);
    connect(p->pingTimeout, &QTimer::timeout, this, &AsemanClientSocketController::pingTimeout);

    reInit();
}

qint64 AsemanClientSocketController::write(const QByteArray &data)
{
    return p->socket->write(data);
}

QByteArray AsemanClientSocketController::read(qint64 maxlen)
{
    return p->socket->read(maxlen);
}

QByteArray AsemanClientSocketController::readAll()
{
    return p->socket->readAll();
}

bool AsemanClientSocketController::isConnected() const
{
    return (p->socket->state() == QAbstractSocket::ConnectedState);
}

bool AsemanClientSocketController::atEnd() const
{
    return p->socket->atEnd();
}

bool AsemanClientSocketController::connecting() const
{
    return p->connecting;
}

void AsemanClientSocketController::wake()
{
    if(p->waked)
        return;

    p->waked = true;
    reconnect();
    refreshConnecting();
}

void AsemanClientSocketController::sleep()
{
    if(!p->waked)
        return;

    p->waked = false;
    reInit();
    p->socketTimeout->stop();
    p->pingTimeout->stop();
    refreshConnecting();
}

bool AsemanClientSocketController::reconnect()
{
    if(!p->waked)
        return false;

    reInit();
    if(p->parent->ssl())
        static_cast<QSslSocket*>(p->socket.data())->connectToHostEncrypted(p->parent->hostAddress(), static_cast<quint16>(p->parent->port()));
    else
        p->socket->connectToHost(p->parent->hostAddress(), static_cast<quint16>(p->parent->port()));

    p->socketTimeout->stop();
    p->socketTimeout->start();

    p->pingTimeout->stop();
    p->pingTimeout->start();

    return true;
}

void AsemanClientSocketController::reInit()
{
    if(p->socket)
        delete p->socket;

    if(p->parent->ssl())
    {
        QSslSocket *sslSocket = new QSslSocket(this);
        sslSocket->setProtocol(QSsl::TlsV1_2);

        p->socket = sslSocket;
        connect(sslSocket, &QSslSocket::encrypted, this, &AsemanClientSocketController::connected);
        connect(sslSocket, static_cast<void(QSslSocket::*)(const QList<QSslError> &)>(&QSslSocket::sslErrors), this,
              [=](const QList<QSslError> &errors){
            for(const QSslError &err: errors)
                qDebug() << "AsemanClientSocketController: Error:" << err;
            static_cast<QSslSocket*>(p->socket.data())->ignoreSslErrors();
        });
    }
    else
    {
        QTcpSocket *socket = new QTcpSocket(this);
        connect(socket, &QTcpSocket::connected, this, &AsemanClientSocketController::connected);

        p->socket = socket;
    }

    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");

    connect(p->socket, &QTcpSocket::readyRead, this, &AsemanClientSocketController::readyRead_prv);
    connect(p->socket, static_cast<void (QTcpSocket::*)(QTcpSocket::SocketError)>(&QTcpSocket::error), this, &AsemanClientSocketController::error, Qt::QueuedConnection);
    connect(p->socket, &QTcpSocket::disconnected, this, &AsemanClientSocketController::disconnected_prv, Qt::QueuedConnection);
    connect(p->socket, &QTcpSocket::stateChanged, this, &AsemanClientSocketController::refreshConnecting);
}

void AsemanClientSocketController::readyRead_prv()
{
    p->socketTimeout->stop();
    p->socketTimeout->start();

    p->pingTimeout->stop();
    p->pingTimeout->start();

    Q_EMIT readyRead();
}

void AsemanClientSocketController::error_prv(QAbstractSocket::SocketError socketError)
{
    QString text;
    switch(static_cast<int>(socketError))
    {
    case QAbstractSocket::ConnectionRefusedError:
        text = QStringLiteral("The connection was refused by the peer (or timed out).");
        reconnect();
        break;

    case QAbstractSocket::RemoteHostClosedError:
        text = QStringLiteral("The remote host closed the connection.");
        reconnect();
        break;

    case QAbstractSocket::HostNotFoundError:
        text = QStringLiteral("The host address was not found.");
        break;

    case QAbstractSocket::SocketAccessError:
        text = QStringLiteral("The socket operation failed because the application lacked the required privileges.");
        break;

    case QAbstractSocket::SocketResourceError:
        text = QStringLiteral("The local system ran out of resources.");
        break;

    case QAbstractSocket::SocketTimeoutError:
        text = QStringLiteral("The socket operation timed out.");
        break;

    case QAbstractSocket::DatagramTooLargeError:
        text = QStringLiteral("The datagram was larger than the operating system's limit.");
        break;

    case QAbstractSocket::NetworkError:
        text = QStringLiteral("An error occurred with the network.");
        disconnected();
        break;

    case QAbstractSocket::AddressInUseError:
        text = QStringLiteral("The bound address is already in use and was set to be exclusive.");
        break;

    case QAbstractSocket::SocketAddressNotAvailableError:
        text = QStringLiteral("The bound address does not belong to the host.");
        break;

    case QAbstractSocket::UnsupportedSocketOperationError:
        text = QStringLiteral("The requested socket operation is not supported by the local operating system.");
        break;

    case QAbstractSocket::UnfinishedSocketOperationError:
        text = QStringLiteral("The last operation attempted has not finished yet (still in progress in the background).");
        break;

    case QAbstractSocket::ProxyAuthenticationRequiredError:
        text = QStringLiteral("The socket is using a proxy, and the proxy requires authentication.");
        break;

    case QAbstractSocket::SslHandshakeFailedError:
        text = QStringLiteral("The SSL/TLS handshake failed, so the connection was closed");
        break;

    case QAbstractSocket::ProxyConnectionRefusedError:
        text = QStringLiteral("Could not contact the proxy server because the connection to that server was denied.");
        break;

    case QAbstractSocket::ProxyConnectionClosedError:
        text = QStringLiteral("The connection to the proxy server was closed unexpectedly (before the connection to the final peer was established)");
        break;

    case QAbstractSocket::ProxyConnectionTimeoutError:
        text = QStringLiteral("The connection to the proxy server timed out or the proxy server stopped responding in the authentication phase.");
        break;

    case QAbstractSocket::ProxyNotFoundError:
        text = QStringLiteral("The proxy address was not found.");
        break;

    case QAbstractSocket::ProxyProtocolError:
        text = QStringLiteral("The connection negotiation with the proxy server failed, because the response from the proxy server could not be understood.");
        break;

    case QAbstractSocket::OperationError:
        text = QStringLiteral("An operation was attempted while the socket was in a state that did not permit it.");
        break;

    case QAbstractSocket::SslInternalError:
        text = QStringLiteral("The SSL library being used reported an internal error. This is probably the result of a bad installation or misconfiguration of the library.");
        break;

    case QAbstractSocket::SslInvalidUserDataError:
        text = QStringLiteral("Invalid data (certificate, key, cypher, etc.) was provided and its use resulted in an error in the SSL library.");
        break;

    case QAbstractSocket::TemporaryError:
        text = QStringLiteral("A temporary error occurred.");
        break;

    case QAbstractSocket::UnknownSocketError:
        text = QStringLiteral("An unidentified error occurred.");
        break;
    }

    Q_EMIT generalError(text);
}

void AsemanClientSocketController::disconnected_prv()
{
    reconnect();
    Q_EMIT disconnected();
}

void AsemanClientSocketController::socketTimeout()
{
    reconnect();
    Q_EMIT timedOut();
}

void AsemanClientSocketController::pingTimeout()
{
    p->pingCounter++;
    p->parent->hardPing(p->pingCounter);
    p->pingTimeout->start();
}

void AsemanClientSocketController::refreshConnecting()
{
    bool connecting = p->waked && !isConnected();
    if(connecting == p->connecting)
        return;

    p->connecting = connecting;
    Q_EMIT connectingChanged();
}

AsemanClientSocketController::~AsemanClientSocketController()
{
    delete p;
}
