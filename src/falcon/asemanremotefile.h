#ifndef ASEMANREMOTEFILE_H
#define ASEMANREMOTEFILE_H

#include <QObject>
#include <QUrl>

#include "asemanabstractclientsocket.h"
#include "asemanclientlib_global.h"

class ASEMANCLIENTLIBSHARED_EXPORT AsemanRemoteFile : public QObject
{
    Q_OBJECT
    Q_ENUMS(ReceiveMethod)
    class Private;

    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QUrl destination READ destination WRITE setDestination NOTIFY destinationChanged)
    Q_PROPERTY(QString destinationFileId READ destinationFileId WRITE setDestinationFileId NOTIFY destinationFileIdChanged)
    Q_PROPERTY(qint32 receiveMethod READ receiveMethod WRITE setReceiveMethod NOTIFY receiveMethodChanged)
    Q_PROPERTY(AsemanAbstractClientSocket* socket READ socket WRITE setSocket NOTIFY socketChanged)
    Q_PROPERTY(QVariantMap details READ details NOTIFY detailsChanged)
    Q_PROPERTY(qint64 size READ size NOTIFY sizeChanged)
    Q_PROPERTY(qreal progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(bool downloading READ downloading NOTIFY downloadingChanged)
    Q_PROPERTY(qint64 startOffset READ startOffset WRITE setStartOffset NOTIFY startOffsetChanged)
    Q_PROPERTY(QUrl finalPath READ finalPath NOTIFY finalPathChanged)

public:
    enum ReceiveMethod {
        ReceiveMediaNormal,
        ReceiveMediaBigFile
    };

    AsemanRemoteFile(QObject *parent = Q_NULLPTR);
    virtual ~AsemanRemoteFile();

    QUrl source() const;
    void setSource(const QUrl &source);

    QUrl destination() const;
    void setDestination(const QUrl &destination);

    QString destinationFileId() const;
    void setDestinationFileId(const QString &destinationFileId);

    qint32 receiveMethod() const;
    void setReceiveMethod(qint32 receiveMethod);

    QUrl finalPath() const;

    void setStartOffset(qint64 startOffset);
    qint64 startOffset() const;

    AsemanAbstractClientSocket *socket() const;
    void setSocket(AsemanAbstractClientSocket *socket);

    QVariantMap details() const;
    bool downloading() const;
    qint64 size() const;
    qreal progress() const;

public Q_SLOTS:
    void start();
    void stop();

Q_SIGNALS:
    void sourceChanged();
    void destinationChanged();
    void receiveMethodChanged();
    void socketChanged();
    void detailsChanged();
    void downloadingChanged();
    void sizeChanged();
    void progressChanged();
    void startOffsetChanged();
    void finalPathChanged();
    void destinationFileIdChanged();
    void error(qint32 errorCode, const QVariant &errorValue);

protected:
    void refresh();
    QString hash() const;

private:
    Private *p;
};

#endif // ASEMANREMOTEFILE_H
