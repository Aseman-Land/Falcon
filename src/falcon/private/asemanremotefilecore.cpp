#include "asemanremotefilecore.h"
#include "services/file1.h"

#include <QPointer>
#include <QDebug>
#include <QTimer>
#include <QFileInfo>
#include <QVariantMap>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class AsemanRemoteFileCore::Private
{
public:
    class FilePathUnit
    {
    public:
        FilePathUnit() : remote(false), http(false) {}
        QString path;
        QString id;
        bool remote;
        bool http;

        static FilePathUnit fromUrl(const QUrl &url);
    };

    class BigFileUnit
    {
    public:
        BigFileUnit() : offset(0), size(0), limit(0) {}
        QString src;
        QString dst;
        qint64 offset;
        qint64 size;
        qint64 limit;
        QPointer<QFile> file;
    };

    QTimer *refreshTimer;
    File1 *file;
    QPointer<AsemanAbstractClientSocket> socket;

    QUrl source;
    QUrl destination;
    QUrl finalPath;
    QString destinationFileId;

    QVariantMap details;
    bool downloading;
    qint64 downloadedSize;
    qint64 startOffset;
    qint32 receiveMethod;

    QHash<qint64, BigFileUnit> bigFileUnits;
    QHash<QNetworkReply*, QByteArray> replyBuffers;
};

AsemanRemoteFileCore::AsemanRemoteFileCore(QObject *parent) :
    QObject(parent)
{
    p = new Private;
    p->downloading = false;
    p->downloadedSize = 0;
    p->startOffset = 0;
    p->receiveMethod = ReceiveMediaBigFile;

    p->file = new File1(this);

    p->refreshTimer = new QTimer(this);
    p->refreshTimer->setInterval(200);
    p->refreshTimer->setSingleShot(true);

    connect(p->refreshTimer, &QTimer::timeout, this, &AsemanRemoteFileCore::refresh);
    connect(p->file, &File1::getFileBigPartResult, this, &AsemanRemoteFileCore::getFileBigPartResult);
}

QUrl AsemanRemoteFileCore::source() const
{
    return p->source;
}

void AsemanRemoteFileCore::setSource(const QUrl &source)
{
    if(p->source == source)
        return;

    p->source = source;
    start();
    Q_EMIT sourceChanged();
}

QUrl AsemanRemoteFileCore::destination() const
{
    return p->destination;
}

void AsemanRemoteFileCore::setDestination(const QUrl &destination)
{
    if(p->destination == destination)
        return;

    p->destination = destination;
    setFinalPath(p->destination);
    start();
    Q_EMIT destinationChanged();
}

QString AsemanRemoteFileCore::destinationFileId() const
{
    return p->destinationFileId;
}

void AsemanRemoteFileCore::setDestinationFileId(const QString &destinationFileId)
{
    if(p->destinationFileId == destinationFileId)
        return;

    p->destinationFileId = destinationFileId;
    start();
    Q_EMIT destinationFileIdChanged();
}

qint32 AsemanRemoteFileCore::receiveMethod() const
{
    return p->receiveMethod;
}

void AsemanRemoteFileCore::setReceiveMethod(qint32 receiveMethod)
{
    if(p->receiveMethod == receiveMethod)
        return;

    p->receiveMethod = receiveMethod;
    Q_EMIT receiveMethodChanged();
}

QUrl AsemanRemoteFileCore::finalPath() const
{
    return p->finalPath;
}

void AsemanRemoteFileCore::setFinalPath(const QUrl &finalPath)
{
    if(p->finalPath == finalPath)
        return;

    p->finalPath = finalPath;
    Q_EMIT finalPathChanged();
}

void AsemanRemoteFileCore::setStartOffset(qint64 startOffset)
{
    if(p->startOffset == startOffset)
        return;

    p->startOffset = startOffset;
    Q_EMIT startOffsetChanged();
}

qint64 AsemanRemoteFileCore::startOffset() const
{
    return p->startOffset;
}

AsemanAbstractClientSocket *AsemanRemoteFileCore::socket() const
{
    return p->socket;
}

void AsemanRemoteFileCore::setSocket(AsemanAbstractClientSocket *socket)
{
    if(p->socket == socket)
        return;

    p->socket = socket;
    start();
    Q_EMIT socketChanged();
}

QVariantMap AsemanRemoteFileCore::details() const
{
    return p->details;
}

bool AsemanRemoteFileCore::downloading() const
{
    return p->downloading;
}

qint64 AsemanRemoteFileCore::size() const
{
    return p->details.value(QStringLiteral("size")).toLongLong();
}

qreal AsemanRemoteFileCore::progress() const
{
    qint64 size = AsemanRemoteFileCore::size();
    if(size)
        return 1.0*p->downloadedSize/size;
    else
        return 0;
}

void AsemanRemoteFileCore::setDownloading(bool downloading)
{
    if(p->downloading == downloading)
        return;

    p->downloading = downloading;
    Q_EMIT downloadingChanged();
}

void AsemanRemoteFileCore::setDownloadedSize(qint64 downloadedSize)
{
    if(p->downloadedSize == downloadedSize)
        return;

    p->downloadedSize = downloadedSize;
    Q_EMIT progressChanged();
}

void AsemanRemoteFileCore::setDetails(const QVariantMap &details)
{
    if(p->details == details)
        return;

    p->details = details;
    Q_EMIT detailsChanged();
    Q_EMIT sizeChanged();
}

void AsemanRemoteFileCore::start()
{
    p->refreshTimer->stop();
    p->refreshTimer->start();
}

void AsemanRemoteFileCore::stop()
{
    p->refreshTimer->stop();
    setDownloading(false);
}

void AsemanRemoteFileCore::refresh()
{
    if(!p->socket || !p->file)
        return;

    Private::FilePathUnit src = Private::FilePathUnit::fromUrl(p->source);
    Private::FilePathUnit dst = Private::FilePathUnit::fromUrl(p->destination);
    if(p->destinationFileId.count())
    {
        dst.id = p->destinationFileId;
        dst.remote = true;
    }

    if(src.path.isEmpty() || (dst.path.isEmpty() && dst.id.isEmpty()))
        return;

    if(src.remote && !dst.remote) {
        if(src.http)
            downloadHttpToLocal(src.path, dst.path);
        else
            downloadRemoteToLocal(src.path, dst.path);
    } else if(!src.remote && dst.remote) {
        if(dst.id.count())
            uploadLocalToRemoteWithId(src.path, dst.id);
        else
            uploadLocalToRemote(src.path, dst.path);
    } else if(!src.remote && !dst.remote) {
        setDownloading(true);
        QFile::copy(src.path, dst.path);
        setFinalPath(p->destination);
        Q_EMIT finalPathChanged();
        setDownloading(false);
    } else {

    }
}

void AsemanRemoteFileCore::downloadRemoteToLocal(const QString &src, const QString &dst)
{
    if(!p->socket || !p->file)
        return;

    setDownloading(true);

    if(QFileInfo::exists(dst))
    {
        setFinalPath(p->destination);
        Q_EMIT finalPathChanged();
    }

    p->file->setSocket(p->socket);
    p->file->getFileDetails(src, this, [this, src, dst](ASEMAN_AGENT_GETFILEDETAILS_CALLBACK){
        Q_UNUSED(id)
        setDetails(result);
        setDownloadedSize(0);

        if(!error.null)
        {
            setDownloading(false);
            Q_EMIT AsemanRemoteFileCore::error(error.errorCode, error.errorValue);
            return;
        }

        qint64 size = result.value(QStringLiteral("size")).toLongLong();
        if(!size)
        {
            setDownloading(false);
            setFinalPath(p->destination);
            return;
        }

        QFileInfo dstFile(dst);
        const qint64 dstSize = dstFile.size();
        if(dstSize == size)
        {
            setDownloading(false);
            setDownloadedSize(dstSize);
        }
        else
        {
            setFinalPath(QUrl());
            qint64 offset = (dstSize>p->startOffset? dstSize : p->startOffset);
            getFilePart(src, dst, offset, size);
        }
    });
}

void AsemanRemoteFileCore::downloadHttpToLocal(const QString &src, const QString &dst)
{
    setDownloading(true);

    QFile *file = new QFile(dst, this);

    if(QFileInfo::exists(dst))
    {
        setFinalPath(p->destination);
        Q_EMIT finalPathChanged();
    }

    QNetworkAccessManager *nm = new QNetworkAccessManager(this);

    QNetworkRequest request;
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    request.setUrl( QUrl(src) );

    QNetworkReply *reply = nm->get(request);

    connect(reply, &QNetworkReply::metaDataChanged, this, [this, reply, dst, file](){
        qint64 size = reply->rawHeader("Content-Length").toLongLong();

        QVariantMap details;
        details[QStringLiteral("size")] = size;

        setDetails(details);

        QFileInfo dstFile(dst);
        const qint64 dstSize = dstFile.size();
        if(dstSize == size)
        {
            setDownloading(false);
            setDownloadedSize(dstSize);
            reply->abort();
            if(file->isOpen())
                file->close();
        }
        else
        {
            setFinalPath(QUrl());
            if(!file->isOpen())
                file->open(QFile::WriteOnly);
            if(p->replyBuffers.contains(reply))
                file->write( p->replyBuffers.take(reply) );
        }
    });
    connect(reply, &QNetworkReply::readyRead, this, [this, reply, file](){
        QByteArray data = reply->readAll();
        if( size() )
        {
            if(!file->isOpen())
                file->open(QFile::WriteOnly);

            file->write(data);
        }
        else
            p->replyBuffers[reply] += data;

        setDownloadedSize( p->downloadedSize + data.size() );
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply, file, nm](){
        if(file->isOpen())
        {
            file->flush();
            file->close();
        }
        file->deleteLater();
        reply->deleteLater();
        nm->deleteLater();
        setDownloading(false);
        setFinalPath( QUrl(p->destination) );
    });
}

void AsemanRemoteFileCore::getFilePart(const QString &src, const QString &dst, qint64 offset, qint64 size, QFile *file)
{
    if(!file)
    {
        file = new QFile(this);
        file->setFileName(dst);
        if(!file->open(QFile::ReadWrite))
        {
            delete file;
            setDownloading(false);
            return;
        }
        if(file->exists())
            file->seek(file->size());
    }
    if(!downloading())
    {
        delete file;
        return;
    }
    if(offset >= size)
    {
        setDownloadedSize(size);
        delete file;
        setFinalPath(p->destination);
        setDownloading(false);
        return;
    }

    const qint32 limit = 1<<17;

    Private::BigFileUnit unit;
    unit.src = src;
    unit.dst = dst;
    unit.file = file;
    unit.limit = limit;
    unit.offset = offset;
    unit.size = size;

    switch(p->receiveMethod)
    {
    case ReceiveMediaBigFile:
        p->file->getFileBigPart(src, offset, static_cast<qint32>(size), this, [this, unit](ASEMAN_AGENT_GETFILEBIGPART_CALLBACK){
            Q_UNUSED(id)
            if(!error.null)
            {
                setDownloading(false);
                unit.file->remove();
                delete unit.file;
                Q_EMIT AsemanRemoteFileCore::error(error.errorCode, error.errorValue);
                return;
            }

            p->bigFileUnits[result] = unit;
        });
        break;

    default:
    case ReceiveMediaNormal:
        p->file->getFilePart(src, offset, limit, this, [this, src, dst, offset, size, file](ASEMAN_AGENT_GETFILEPART_CALLBACK){
            Q_UNUSED(id)
            if(!error.null)
            {
                setDownloading(false);
                file->remove();
                delete file;
                Q_EMIT AsemanRemoteFileCore::error(error.errorCode, error.errorValue);
                return;
            }

            file->seek(offset);
            file->write(result);
            file->flush();

            qint64 newOffset = offset + result.size();
            setDownloadedSize(newOffset);

            getFilePart(src, dst, newOffset, size, file);
        });
        break;
    }
}

void AsemanRemoteFileCore::uploadLocalToRemoteWithId(const QString &src, const QString &dstId)
{
    if(!p->socket || !p->file)
        return;

    setDownloading(true);

    QPointer<QFile> file = new QFile(src, this);
    if(!file->open(QFile::ReadOnly))
    {
        delete file;
        setDownloading(false);
        return;
    }

    QVariantMap details;
    details[QStringLiteral("size")] = file->size();

    setDetails(details);

    while(!file->atEnd())
    {
        qint64 offset = file->pos();
        QByteArray data = file->read(32*1024);
        qint64 size = offset + data.size();
        bool lastPart = file->atEnd();
        p->file->setSocket(p->socket);
        p->file->writeToFile(dstId, static_cast<qint32>(offset), data, this, [this, file, size](ASEMAN_AGENT_WRITETOFILE_CALLBACK){
            Q_UNUSED(error)
            Q_UNUSED(id)
            Q_UNUSED(result)
            setDownloadedSize(size);
        });
        if(lastPart)
        {
            p->file->finishWriteFile(dstId, this, [this, file](ASEMAN_AGENT_FINISHWRITEFILE_CALLBACK){
                Q_UNUSED(error)
                Q_UNUSED(id)
                Q_UNUSED(result)
                setDownloading(false);
                file->deleteLater();
            });
        }
    }
}

void AsemanRemoteFileCore::uploadFilePartWidthId(const QString &src, const QString &dstId, qint64 size, QFile *file)
{

}

void AsemanRemoteFileCore::uploadLocalToRemote(const QString &src, const QString &dst)
{

}

void AsemanRemoteFileCore::getFileBigPartResult(qint64 id, QByteArray data)
{
    if(!p->bigFileUnits.contains(id))
        return;

    Private::BigFileUnit unit = p->bigFileUnits.take(id);
    if(!unit.file)
    {
        setDownloading(false);
        Q_EMIT error(0, QStringLiteral("Unknown error"));
        return;
    }

    unit.file->seek(unit.offset);
    unit.file->write(data);
    unit.file->flush();

    qint64 newOffset = unit.offset + data.size();
    if(newOffset > unit.size)
    {
        unit.file->close();
        delete unit.file;
        setDownloadedSize(unit.size);
    }
    else
    {
        setDownloadedSize(newOffset);
        unit.offset = newOffset;
        p->bigFileUnits[id] = unit;
    }

    if(data.isEmpty())
        getFilePart(unit.src, unit.dst, newOffset, unit.size, unit.file);
}

AsemanRemoteFileCore::~AsemanRemoteFileCore()
{
    QHashIterator<qint64, Private::BigFileUnit> i(p->bigFileUnits);
    while(i.hasNext())
    {
        i.next();
        p->file->getBigFileCancel(i.key());
    }

    delete p;
}



AsemanRemoteFileCore::Private::FilePathUnit AsemanRemoteFileCore::Private::FilePathUnit::fromUrl(const QUrl &url)
{
    FilePathUnit res;
    res.path = url.toLocalFile();
    if(res.path.length())
        return res;

    res.path = url.toString();
    if(res.path.length())
    {
        if(res.path.left(9) == QStringLiteral("aseman://"))
        {
            res.path = res.path.mid(9);
            res.http = false;
            res.remote = true;
        }
        else
        if(res.path.left(7) == QStringLiteral("http://") || res.path.left(8) == QStringLiteral("https://"))
        {
            res.http = true;
            res.remote = true;
        }
        else
            res.path.clear();
    }

    return res;
}
