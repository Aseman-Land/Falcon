#ifndef ASEMANIODEVICE_H
#define ASEMANIODEVICE_H

#include <QIODevice>
#include "asemanabstractclientsocket.h"
#include "asemanclientlib_global.h"

class ASEMANCLIENTLIBSHARED_EXPORT AsemanIODevice : public QIODevice
{
    Q_OBJECT
    class Private;

public:
    AsemanIODevice(QObject *parent = Q_NULLPTR);
    AsemanIODevice(AsemanAbstractClientSocket *socket, const QString &source, QObject *parent = Q_NULLPTR);
    virtual ~AsemanIODevice();

    qint64 pos() const;
    qint64 size() const;
    bool seek(qint64 pos);
    bool atEnd() const;
    bool reset();

    bool open(OpenMode mode);
    void close();

    bool isSequential() const;

    qint64 bytesAvailable() const;
    bool waitForReadyRead(int msecs);

Q_SIGNALS:
    void error(qint64 code, const QVariant &value);

public Q_SLOTS:
    void setSource(AsemanAbstractClientSocket *socket, const QString &source);

protected:
    void rebuffer();
    void getFileBigPartResult(qint64 id, QByteArray data);

    virtual qint64 readData(char *data, qint64 maxlen);
    virtual qint64 writeData(const char *data, qint64 len);

private:
    Private *p;
};

#endif // ASEMANIODEVICE_H
