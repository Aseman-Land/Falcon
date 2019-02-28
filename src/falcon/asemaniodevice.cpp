#include "asemaniodevice.h"
#include "services/file1.h"

#include <QTimer>
#include <QEventLoop>

#include <QDebug>

class AsemanIODevice::Private
{
public:
    QByteArray buffer;
    qint64 bufferSize;
    qint64 bufferOffset;
    qint64 bufferPosition;
    qint64 fileSize;
    QVariantMap fileDetails;

    File1 *file;
    QString source;

    qint64 currentBufferingId;
};

AsemanIODevice::AsemanIODevice(QObject *parent) :
    QIODevice(parent)
{
    p = new Private;
    p->currentBufferingId = 0;
    p->bufferSize = (1<<23);
    p->bufferOffset = 0;
    p->bufferPosition = 0;

    p->fileSize = 0;
    p->file = new File1(this);

    connect(p->file, &File1::getFileBigPartResult, this, &AsemanIODevice::getFileBigPartResult);
}

AsemanIODevice::AsemanIODevice(AsemanAbstractClientSocket *socket, const QString &source, QObject *parent) :
    AsemanIODevice(parent)
{
    setSource(socket, source);
}

qint64 AsemanIODevice::pos() const
{
    return p->bufferOffset + p->bufferPosition;
}

qint64 AsemanIODevice::size() const
{
    return p->fileSize;
}

bool AsemanIODevice::seek(qint64 pos)
{
    if(pos < 0 || p->fileSize < pos)
        return false;

    if(p->bufferOffset <= pos && pos < p->bufferOffset + p->bufferSize)
    {
        p->bufferPosition = (pos - p->bufferOffset);
        if(p->bufferPosition > p->bufferSize/2)
            rebuffer();
    }
    else
    {
        p->bufferOffset = pos;
        p->bufferPosition = 0;
        p->buffer.clear();
        rebuffer();
    }

    return true;
}

bool AsemanIODevice::atEnd() const
{
    return pos() >= size();
}

bool AsemanIODevice::reset()
{
    close();
    rebuffer();
    return true;
}

bool AsemanIODevice::open(QIODevice::OpenMode mode)
{
    QIODevice::open(mode);
    switch( static_cast<qint32>(mode) )
    {
    case QIODevice::ReadOnly:
        reset();
        return true;

    default:
        return false;
    }
}

void AsemanIODevice::close()
{
    p->buffer.clear();
    p->currentBufferingId = 0;
    p->bufferOffset = 0;
    p->bufferPosition = 0;
}

bool AsemanIODevice::isSequential() const
{
    return false;
}

qint64 AsemanIODevice::bytesAvailable() const
{
    return p->buffer.size() + QIODevice::bytesAvailable();
}

bool AsemanIODevice::waitForReadyRead(int msecs)
{
    bool res = true;
    QTimer timer;
    QEventLoop loop;

    connect(&timer, &QTimer::timeout, this, [&loop, &res](){
        loop.exit();
        res = false;
    });

    timer.setSingleShot(true);
    timer.start(msecs);
    loop.exec(QEventLoop::ExcludeUserInputEvents);
    return res;
}

void AsemanIODevice::setSource(AsemanAbstractClientSocket *socket, const QString &source)
{
    p->source = source;
    p->file->setSocket(socket);
    p->file->getFileDetails(p->source, this, [this](ASEMAN_AGENT_GETFILEDETAILS_CALLBACK){
        Q_UNUSED(id)
        if(!error.null)
        {
            setErrorString(error.errorValue.toString());
            Q_EMIT AsemanIODevice::error(error.errorCode, error.errorValue);
            return;
        }
        p->fileDetails = result;
        p->fileSize = p->fileDetails.value(QStringLiteral("size")).toLongLong();
        rebuffer();
    });
    close();
}

void AsemanIODevice::rebuffer()
{
    p->currentBufferingId = 0;
    p->buffer = p->buffer.mid(static_cast<qint32>(p->bufferPosition));
    p->file->getFileBigPart(p->source, p->bufferOffset + p->bufferPosition, static_cast<qint32>(p->bufferSize), this, [this](ASEMAN_AGENT_GETFILEBIGPART_CALLBACK){
        Q_UNUSED(id)
        if(!error.null)
        {
            setErrorString(error.errorValue.toString());
            Q_EMIT AsemanIODevice::error(error.errorCode, error.errorValue);
            return;
        }

        p->currentBufferingId = result;
    });
}

void AsemanIODevice::getFileBigPartResult(qint64, QByteArray data)
{/*
    if(p->currentBufferingId != id)
        return;*/

    p->buffer += data;
    if(data.isEmpty())
        p->currentBufferingId = 0;

    Q_EMIT readyRead();
}

qint64 AsemanIODevice::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data)
    Q_UNUSED(len)
    return -1;
}

qint64 AsemanIODevice::readData(char *data, qint64 maxlen)
{
    if(atEnd())
        return -1;

    QByteArray src = p->buffer.mid(static_cast<qint32>(pos()), static_cast<qint32>(maxlen));
    memcpy(data, src.constData(), static_cast<size_t>(src.size()));
    return src.size();
}

AsemanIODevice::~AsemanIODevice()
{
    delete p;
}
