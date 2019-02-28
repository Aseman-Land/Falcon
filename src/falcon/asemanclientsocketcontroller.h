#ifndef ASEMANCLIENTSOCKETCONTROLLER_H
#define ASEMANCLIENTSOCKETCONTROLLER_H

#include <QObject>
#include <QTcpSocket>

#include "asemanclientlib_global.h"

class ASEMANCLIENTLIBSHARED_EXPORT AsemanClientSocketController : public QObject
{
    Q_OBJECT
    class Private;
    friend class AsemanClientSocket;

public:
    AsemanClientSocketController(class AsemanClientSocket *parent = Q_NULLPTR);
    virtual ~AsemanClientSocketController();

    void setSsl(bool sll);

    qint64 write(const QByteArray &data);
    QByteArray read(qint64 maxlen);
    QByteArray readAll();

    bool isConnected() const;
    bool atEnd() const;
    bool connecting() const;

Q_SIGNALS:
    void readyRead();
    void error(QAbstractSocket::SocketError);
    void connected();
    void connectingChanged();
    void disconnected();
    void timedOut();
    void generalError(const QString &text);

public Q_SLOTS:
    void wake();
    void sleep();
    bool reconnect();

protected:
    void reInit();

private:
    void readyRead_prv();
    void error_prv(QAbstractSocket::SocketError socketError);
    void disconnected_prv();
    void socketTimeout();
    void pingTimeout();
    void refreshConnecting();

private:
    Private *p;
};

#endif // ASEMANCLIENTSOCKETCONTROLLER_H
