#include "asemanremotefile.h"
#include "private/asemanremotefilecore.h"
#include "services/file1.h"

#include <QPointer>
#include <QDebug>
#include <QTimer>
#include <QFileInfo>
#include <QVariantMap>
#include <QDataStream>
#include <QCoreApplication>

class AsemanRemoteFile::Private
{
public:
    QTimer *refreshTimer;
    QPointer<AsemanAbstractClientSocket> socket;

    QUrl source;
    QUrl destination;
    QUrl finalPath;
    QString destinationFileId;
    qint64 startOffset;
    qint32 receiveMethod;

    QPointer<AsemanRemoteFileCore> currentCore;
    QString currentHash;

    static QHash<QString, QPair<AsemanRemoteFileCore*, QSet<QObject*> > > cores;
};

QHash<QString, QPair<AsemanRemoteFileCore*, QSet<QObject*> > > AsemanRemoteFile::Private::cores;

AsemanRemoteFile::AsemanRemoteFile(QObject *parent) :
    QObject(parent)
{
    p = new Private;
    p->startOffset = 0;
    p->receiveMethod = ReceiveMediaBigFile;

    p->refreshTimer = new QTimer(this);
    p->refreshTimer->setInterval(200);
    p->refreshTimer->setSingleShot(true);

    connect(p->refreshTimer, &QTimer::timeout, this, &AsemanRemoteFile::refresh);
}

QUrl AsemanRemoteFile::source() const
{
    return p->source;
}

void AsemanRemoteFile::setSource(const QUrl &source)
{
    if(p->source == source)
        return;

    p->source = source;
    start();
    Q_EMIT sourceChanged();
}

QUrl AsemanRemoteFile::destination() const
{
    return p->destination;
}

void AsemanRemoteFile::setDestination(const QUrl &destination)
{
    if(p->destination == destination)
        return;

    p->destination = destination;
    start();
    Q_EMIT destinationChanged();
}

QString AsemanRemoteFile::destinationFileId() const
{
    return p->destinationFileId;
}

void AsemanRemoteFile::setDestinationFileId(const QString &destinationFileId)
{
    if(p->destinationFileId == destinationFileId)
        return;

    p->destinationFileId = destinationFileId;
    start();
    Q_EMIT destinationFileIdChanged();
}

qint32 AsemanRemoteFile::receiveMethod() const
{
    return p->receiveMethod;
}

void AsemanRemoteFile::setReceiveMethod(qint32 receiveMethod)
{
    if(p->receiveMethod == receiveMethod)
        return;

    p->receiveMethod = receiveMethod;
    Q_EMIT receiveMethodChanged();
}

QUrl AsemanRemoteFile::finalPath() const
{
    if(!p->currentCore)
        return QString();

    return p->currentCore->finalPath();
}

QString AsemanRemoteFile::hash() const
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream << p->source;
    stream << p->destination;
    stream << p->destinationFileId;
    stream << reinterpret_cast<qint64>(p->socket.data());
    stream << p->startOffset;
    stream << p->receiveMethod;

    return QString::fromUtf8( QCryptographicHash::hash(bytes, QCryptographicHash::Md5).toHex() );
}

void AsemanRemoteFile::setStartOffset(qint64 startOffset)
{
    if(p->startOffset == startOffset)
        return;

    p->startOffset = startOffset;
    Q_EMIT startOffsetChanged();
}

qint64 AsemanRemoteFile::startOffset() const
{
    return p->startOffset;
}

AsemanAbstractClientSocket *AsemanRemoteFile::socket() const
{
    return p->socket;
}

void AsemanRemoteFile::setSocket(AsemanAbstractClientSocket *socket)
{
    if(p->socket == socket)
        return;

    p->socket = socket;
    start();
    Q_EMIT socketChanged();
}

QVariantMap AsemanRemoteFile::details() const
{
    if(!p->currentCore)
        return QVariantMap();

    return p->currentCore->details();
}

bool AsemanRemoteFile::downloading() const
{
    if(!p->currentCore)
        return false;

    return p->currentCore->downloading();
}

qint64 AsemanRemoteFile::size() const
{
    if(!p->currentCore)
        return 0;

    return p->currentCore->size();
}

qreal AsemanRemoteFile::progress() const
{
    if(!p->currentCore)
        return 0;

    return p->currentCore->progress();
}

void AsemanRemoteFile::start()
{
    p->refreshTimer->stop();
    p->refreshTimer->start();
}

void AsemanRemoteFile::stop()
{
    if(!p->currentCore)
        return;

    p->currentCore->stop();
}

void AsemanRemoteFile::refresh()
{
    if(p->currentCore)
        return;

    QString hash = AsemanRemoteFile::hash();

    QPair<AsemanRemoteFileCore*, QSet<QObject*> > &core = AsemanRemoteFile::Private::cores[hash];
    if(core.second.contains(this))
        return;

    core.second.insert(this);

    if(!core.first)
    {
        AsemanRemoteFileCore *coreObj = new AsemanRemoteFileCore();

        core.first = coreObj;
        core.first->setSource(p->source);
        core.first->setDestination(p->destination);
        core.first->setStartOffset(p->startOffset);
        core.first->setReceiveMethod(p->receiveMethod);
        core.first->setDestinationFileId(p->destinationFileId);
        core.first->setSocket(p->socket);

        connect(coreObj, &AsemanRemoteFileCore::downloadingChanged, [coreObj]{
            if(!coreObj->downloading())
                coreObj->deleteLater();
        });
        connect(coreObj, &AsemanRemoteFileCore::destroyed, [hash]{
            AsemanRemoteFile::Private::cores.remove(hash);
        });
    }

    connect(core.first, &AsemanRemoteFileCore::detailsChanged, this, &AsemanRemoteFile::detailsChanged);
    connect(core.first, &AsemanRemoteFileCore::sizeChanged, this, &AsemanRemoteFile::sizeChanged);
    connect(core.first, &AsemanRemoteFileCore::progressChanged, this, &AsemanRemoteFile::progressChanged);
    connect(core.first, &AsemanRemoteFileCore::finalPathChanged, this, &AsemanRemoteFile::finalPathChanged);
    connect(core.first, &AsemanRemoteFileCore::error, this, &AsemanRemoteFile::error);
    connect(core.first, &AsemanRemoteFileCore::downloadingChanged, this, &AsemanRemoteFile::downloadingChanged);

    p->currentHash = hash;
    p->currentCore = core.first;

    if(core.first->downloading()) Q_EMIT downloadingChanged();
    if(core.first->progress()) Q_EMIT progressChanged();
    if(core.first->size()) Q_EMIT sizeChanged();
    if(core.first->details().count()) Q_EMIT detailsChanged();
    if(!core.first->finalPath().isEmpty()) Q_EMIT finalPathChanged();
}

AsemanRemoteFile::~AsemanRemoteFile()
{
    if(p->currentCore)
    {
        AsemanRemoteFile::Private::cores[p->currentHash].second.remove(this);
        if( AsemanRemoteFile::Private::cores[p->currentHash].second.isEmpty() )
            p->currentCore->stop();
    }

    delete p;
}
