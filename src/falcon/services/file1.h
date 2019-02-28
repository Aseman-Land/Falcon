#ifndef FILE1_H
#define FILE1_H

#include <asemanabstractagentclient.h>

#define ASEMAN_AGENT_PING_CALLBACK ASEMAN_AGENT_CALLBACK(QString)
#define ASEMAN_AGENT_GETFILEPART_CALLBACK ASEMAN_AGENT_CALLBACK(QByteArray)
#define ASEMAN_AGENT_GETFILEBIGPART_CALLBACK ASEMAN_AGENT_CALLBACK(qlonglong)
#define ASEMAN_AGENT_GETBIGFILECANCEL_CALLBACK ASEMAN_AGENT_CALLBACK(bool)
#define ASEMAN_AGENT_GETFILEDETAILS_CALLBACK ASEMAN_AGENT_CALLBACK(QVariantMap)
#define ASEMAN_AGENT_WRITETOFILE_CALLBACK ASEMAN_AGENT_CALLBACK(bool)
#define ASEMAN_AGENT_FINISHWRITEFILE_CALLBACK ASEMAN_AGENT_CALLBACK(bool)
#define ASEMAN_AGENT_CHECKPERMISSION_CALLBACK ASEMAN_AGENT_CALLBACK(bool)


#ifdef QT_QML_LIB
#include <QJSValue>
#endif

class File1: public AsemanAbstractAgentClient
{
    Q_OBJECT
    Q_ENUMS(Permission)
    Q_ENUMS(Errors)
    Q_PROPERTY(QString name_ping READ name_ping NOTIFY fakeSignal)
    Q_PROPERTY(QString name_getFilePart READ name_getFilePart NOTIFY fakeSignal)
    Q_PROPERTY(QString name_getFileBigPart READ name_getFileBigPart NOTIFY fakeSignal)
    Q_PROPERTY(QString name_getBigFileCancel READ name_getBigFileCancel NOTIFY fakeSignal)
    Q_PROPERTY(QString name_getFileDetails READ name_getFileDetails NOTIFY fakeSignal)
    Q_PROPERTY(QString name_writeToFile READ name_writeToFile NOTIFY fakeSignal)
    Q_PROPERTY(QString name_finishWriteFile READ name_finishWriteFile NOTIFY fakeSignal)
    Q_PROPERTY(QString name_checkPermission READ name_checkPermission NOTIFY fakeSignal)

public:
    enum Permission {
        PermissionPublicNone = 0x0,
        PermissionPublicRead = 0x1,
        PermissionPublicWrite = 0x2
    };
    enum Errors {
        ErrorFileNotFound = 0x9125BBC,
        ErrorFileIdNotFound = 0x1079F8D,
        ErrorBadOffsettSize = 0x8AF69BC,
        ErrorSeekError = 0x4E4AB10,
        ErrorBigFile = 0x64014E4,
        ErrorCreateFile = 0x2BB2107,
        ErrorFilePermissionDenied = 0xA370C91,
        ErrorFileOpenError = 0x1FC8479
    };

    File1(QObject *parent = Q_NULLPTR) :
        AsemanAbstractAgentClient(parent),
        _service(QStringLiteral("file")),
        _version(1) {
    }
    virtual ~File1() {
    }

    virtual qint64 pushRequest(const QString &method, const QVariantList &args, qint32 priority, bool hasResult) {
        return AsemanAbstractAgentClient::pushRequest(_service, _version, method, args, priority, hasResult);
    }

    QString name_ping() const { return QStringLiteral("ping"); }
    Q_INVOKABLE qint64 ping(int num, QObject *base = Q_NULLPTR, Callback<QString> callBack = Q_NULLPTR, qint32 priority = ASM_DEFAULT_PRIORITY) {
        qint64 id = pushRequest(QStringLiteral("ping"), QVariantList() << QVariant::fromValue<int>(num), priority, true);
        _calls[id] = QStringLiteral("ping");
        pushBase(id, base);
        callBackPush<QString>(id, callBack);
        return id;
    }

    QString name_getFilePart() const { return QStringLiteral("getFilePart"); }
    Q_INVOKABLE qint64 getFilePart(QString path, qlonglong offset, int limit, QObject *base = Q_NULLPTR, Callback<QByteArray> callBack = Q_NULLPTR, qint32 priority = ASM_DEFAULT_PRIORITY) {
        qint64 id = pushRequest(QStringLiteral("getFilePart"), QVariantList() << QVariant::fromValue<QString>(path) << QVariant::fromValue<qlonglong>(offset) << QVariant::fromValue<int>(limit), priority, true);
        _calls[id] = QStringLiteral("getFilePart");
        pushBase(id, base);
        callBackPush<QByteArray>(id, callBack);
        return id;
    }

    QString name_getFileBigPart() const { return QStringLiteral("getFileBigPart"); }
    Q_INVOKABLE qint64 getFileBigPart(QString path, qlonglong offset, int limit, QObject *base = Q_NULLPTR, Callback<qlonglong> callBack = Q_NULLPTR, qint32 priority = ASM_DEFAULT_PRIORITY) {
        qint64 id = pushRequest(QStringLiteral("getFileBigPart"), QVariantList() << QVariant::fromValue<QString>(path) << QVariant::fromValue<qlonglong>(offset) << QVariant::fromValue<int>(limit), priority, true);
        _calls[id] = QStringLiteral("getFileBigPart");
        pushBase(id, base);
        callBackPush<qlonglong>(id, callBack);
        return id;
    }

    QString name_getBigFileCancel() const { return QStringLiteral("getBigFileCancel"); }
    Q_INVOKABLE qint64 getBigFileCancel(qlonglong _id, QObject *base = Q_NULLPTR, Callback<bool> callBack = Q_NULLPTR, qint32 priority = ASM_DEFAULT_PRIORITY) {
        qint64 id = pushRequest(QStringLiteral("getBigFileCancel"), QVariantList() << QVariant::fromValue<qlonglong>(_id), priority, true);
        _calls[id] = QStringLiteral("getBigFileCancel");
        pushBase(id, base);
        callBackPush<bool>(id, callBack);
        return id;
    }

    QString name_getFileDetails() const { return QStringLiteral("getFileDetails"); }
    Q_INVOKABLE qint64 getFileDetails(QString path, QObject *base = Q_NULLPTR, Callback<QVariantMap> callBack = Q_NULLPTR, qint32 priority = ASM_DEFAULT_PRIORITY) {
        qint64 id = pushRequest(QStringLiteral("getFileDetails"), QVariantList() << QVariant::fromValue<QString>(path), priority, true);
        _calls[id] = QStringLiteral("getFileDetails");
        pushBase(id, base);
        callBackPush<QVariantMap>(id, callBack);
        return id;
    }

    QString name_writeToFile() const { return QStringLiteral("writeToFile"); }
    Q_INVOKABLE qint64 writeToFile(QString fileId, int offset, QByteArray data, QObject *base = Q_NULLPTR, Callback<bool> callBack = Q_NULLPTR, qint32 priority = ASM_DEFAULT_PRIORITY) {
        qint64 id = pushRequest(QStringLiteral("writeToFile"), QVariantList() << QVariant::fromValue<QString>(fileId) << QVariant::fromValue<int>(offset) << QVariant::fromValue<QByteArray>(data), priority, true);
        _calls[id] = QStringLiteral("writeToFile");
        pushBase(id, base);
        callBackPush<bool>(id, callBack);
        return id;
    }

    QString name_finishWriteFile() const { return QStringLiteral("finishWriteFile"); }
    Q_INVOKABLE qint64 finishWriteFile(QString fileId, QObject *base = Q_NULLPTR, Callback<bool> callBack = Q_NULLPTR, qint32 priority = ASM_DEFAULT_PRIORITY) {
        qint64 id = pushRequest(QStringLiteral("finishWriteFile"), QVariantList() << QVariant::fromValue<QString>(fileId), priority, true);
        _calls[id] = QStringLiteral("finishWriteFile");
        pushBase(id, base);
        callBackPush<bool>(id, callBack);
        return id;
    }

    QString name_checkPermission() const { return QStringLiteral("checkPermission"); }
    Q_INVOKABLE qint64 checkPermission(QString path, QObject *base = Q_NULLPTR, Callback<bool> callBack = Q_NULLPTR, qint32 priority = ASM_DEFAULT_PRIORITY) {
        qint64 id = pushRequest(QStringLiteral("checkPermission"), QVariantList() << QVariant::fromValue<QString>(path), priority, true);
        _calls[id] = QStringLiteral("checkPermission");
        pushBase(id, base);
        callBackPush<bool>(id, callBack);
        return id;
    }


#ifdef QT_QML_LIB
public Q_SLOTS:
    /*!
     * Callbacks gives result value and error map as arguments.
     */
    qint64 ping(int num, const QJSValue &jsCallback, qint32 priority = ASM_DEFAULT_PRIORITY) {
        return ping(num, this, [this, jsCallback](qint64, const QString &result, const CallbackError &error) {
            callBackJs(jsCallback, result, error);
        }, priority);
    }
    qint64 getFilePart(QString path, qlonglong offset, int limit, const QJSValue &jsCallback, qint32 priority = ASM_DEFAULT_PRIORITY) {
        return getFilePart(path, offset, limit, this, [this, jsCallback](qint64, const QByteArray &result, const CallbackError &error) {
            callBackJs(jsCallback, result, error);
        }, priority);
    }
    qint64 getFileBigPart(QString path, qlonglong offset, int limit, const QJSValue &jsCallback, qint32 priority = ASM_DEFAULT_PRIORITY) {
        return getFileBigPart(path, offset, limit, this, [this, jsCallback](qint64, const qlonglong &result, const CallbackError &error) {
            callBackJs(jsCallback, result, error);
        }, priority);
    }
    qint64 getBigFileCancel(qlonglong _id, const QJSValue &jsCallback, qint32 priority = ASM_DEFAULT_PRIORITY) {
        return getBigFileCancel(_id, this, [this, jsCallback](qint64, const bool &result, const CallbackError &error) {
            callBackJs(jsCallback, result, error);
        }, priority);
    }
    qint64 getFileDetails(QString path, const QJSValue &jsCallback, qint32 priority = ASM_DEFAULT_PRIORITY) {
        return getFileDetails(path, this, [this, jsCallback](qint64, const QVariantMap &result, const CallbackError &error) {
            callBackJs(jsCallback, result, error);
        }, priority);
    }
    qint64 writeToFile(QString fileId, int offset, QByteArray data, const QJSValue &jsCallback, qint32 priority = ASM_DEFAULT_PRIORITY) {
        return writeToFile(fileId, offset, data, this, [this, jsCallback](qint64, const bool &result, const CallbackError &error) {
            callBackJs(jsCallback, result, error);
        }, priority);
    }
    qint64 finishWriteFile(QString fileId, const QJSValue &jsCallback, qint32 priority = ASM_DEFAULT_PRIORITY) {
        return finishWriteFile(fileId, this, [this, jsCallback](qint64, const bool &result, const CallbackError &error) {
            callBackJs(jsCallback, result, error);
        }, priority);
    }
    qint64 checkPermission(QString path, const QJSValue &jsCallback, qint32 priority = ASM_DEFAULT_PRIORITY) {
        return checkPermission(path, this, [this, jsCallback](qint64, const bool &result, const CallbackError &error) {
            callBackJs(jsCallback, result, error);
        }, priority);
    }

#endif //QT_QML_LIB

Q_SIGNALS:
    void fakeSignal();
    void pingAnswer(qint64 id, QString result);
    void pingError(qint64 id, qint32 errorCode, const QVariant &errorValue);
    void getFileBigPartResult(qlonglong _id, QByteArray data);
    void getFilePartAnswer(qint64 id, QByteArray result);
    void getFilePartError(qint64 id, qint32 errorCode, const QVariant &errorValue);
    void getFileBigPartAnswer(qint64 id, qlonglong result);
    void getFileBigPartError(qint64 id, qint32 errorCode, const QVariant &errorValue);
    void getBigFileCancelAnswer(qint64 id, bool result);
    void getBigFileCancelError(qint64 id, qint32 errorCode, const QVariant &errorValue);
    void getFileDetailsAnswer(qint64 id, QVariantMap result);
    void getFileDetailsError(qint64 id, qint32 errorCode, const QVariant &errorValue);
    void writeToFileAnswer(qint64 id, bool result);
    void writeToFileError(qint64 id, qint32 errorCode, const QVariant &errorValue);
    void finishWriteFileAnswer(qint64 id, bool result);
    void finishWriteFileError(qint64 id, qint32 errorCode, const QVariant &errorValue);
    void checkPermissionAnswer(qint64 id, bool result);
    void checkPermissionError(qint64 id, qint32 errorCode, const QVariant &errorValue);

protected:
    void processError(qint64 id, const CallbackError &error) {
        processResult(id, QVariant(), error);
    }

    void processAnswer(qint64 id, const QVariant &result) {
        processResult(id, result, CallbackError());
    }

    void processResult(qint64 id, const QVariant &result, const CallbackError &error) {
        const QString method = _calls.value(id);
        if(method == QStringLiteral("ping")) {
            callBackCall<QString>(id, result.value<QString>(), error);
            _calls.remove(id);
            if(error.null) Q_EMIT pingAnswer(id, result.value<QString>());
            else Q_EMIT pingError(id, error.errorCode, error.errorValue);
        } else
        if(method == QStringLiteral("getFilePart")) {
            callBackCall<QByteArray>(id, result.value<QByteArray>(), error);
            _calls.remove(id);
            if(error.null) Q_EMIT getFilePartAnswer(id, result.value<QByteArray>());
            else Q_EMIT getFilePartError(id, error.errorCode, error.errorValue);
        } else
        if(method == QStringLiteral("getFileBigPart")) {
            callBackCall<qlonglong>(id, result.value<qlonglong>(), error);
            _calls.remove(id);
            if(error.null) Q_EMIT getFileBigPartAnswer(id, result.value<qlonglong>());
            else Q_EMIT getFileBigPartError(id, error.errorCode, error.errorValue);
        } else
        if(method == QStringLiteral("getBigFileCancel")) {
            callBackCall<bool>(id, result.value<bool>(), error);
            _calls.remove(id);
            if(error.null) Q_EMIT getBigFileCancelAnswer(id, result.value<bool>());
            else Q_EMIT getBigFileCancelError(id, error.errorCode, error.errorValue);
        } else
        if(method == QStringLiteral("getFileDetails")) {
            callBackCall<QVariantMap>(id, result.value<QVariantMap>(), error);
            _calls.remove(id);
            if(error.null) Q_EMIT getFileDetailsAnswer(id, result.value<QVariantMap>());
            else Q_EMIT getFileDetailsError(id, error.errorCode, error.errorValue);
        } else
        if(method == QStringLiteral("writeToFile")) {
            callBackCall<bool>(id, result.value<bool>(), error);
            _calls.remove(id);
            if(error.null) Q_EMIT writeToFileAnswer(id, result.value<bool>());
            else Q_EMIT writeToFileError(id, error.errorCode, error.errorValue);
        } else
        if(method == QStringLiteral("finishWriteFile")) {
            callBackCall<bool>(id, result.value<bool>(), error);
            _calls.remove(id);
            if(error.null) Q_EMIT finishWriteFileAnswer(id, result.value<bool>());
            else Q_EMIT finishWriteFileError(id, error.errorCode, error.errorValue);
        } else
        if(method == QStringLiteral("checkPermission")) {
            callBackCall<bool>(id, result.value<bool>(), error);
            _calls.remove(id);
            if(error.null) Q_EMIT checkPermissionAnswer(id, result.value<bool>());
            else Q_EMIT checkPermissionError(id, error.errorCode, error.errorValue);
        } else
            Q_UNUSED(result);
        if(!error.null) Q_EMIT AsemanAbstractAgentClient::error(id, error.errorCode, error.errorValue);
    }

    void processSignals(const QString &method, const QVariantList &args) {
        if(method == QStringLiteral("getFileBigPartResult")) {
            if(args.count() != 2) return;
            Q_EMIT getFileBigPartResult(args[0].value<qlonglong>(), args[1].value<QByteArray>());
        } else
            Q_UNUSED(args);
    }

private:
    QString _service;
    int _version;
    QHash<qint64, QString> _calls;
};


#endif //FILE1_H

