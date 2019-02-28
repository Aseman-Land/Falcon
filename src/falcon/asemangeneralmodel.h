#ifndef ASEMANGENERALMODEL_H
#define ASEMANGENERALMODEL_H

#include <QAbstractListModel>
#include <QStringList>

#include <asemanabstractagentclient.h>
#include "asemanclientlib_global.h"

class ASEMANCLIENTLIBSHARED_EXPORT AsemanGeneralModel : public QAbstractListModel
{
    Q_OBJECT
    class Private;

    Q_ENUMS(SortMode)

    Q_PROPERTY(AsemanAbstractAgentClient* agent READ agent WRITE setAgent NOTIFY agentChanged)
    Q_PROPERTY(QString method READ method WRITE setMethod NOTIFY methodChanged)
    Q_PROPERTY(QVariantList arguments READ arguments WRITE setArguments NOTIFY argumentsChanged)
    Q_PROPERTY(qint32 count READ count NOTIFY countChanged)
    Q_PROPERTY(qint32 offset READ offset NOTIFY offsetChanged)
    Q_PROPERTY(qint32 sortMode READ sortMode WRITE setSortMode NOTIFY sortModeChanged)
    Q_PROPERTY(qint32 limit READ limit WRITE setLimit NOTIFY limitChanged)
    Q_PROPERTY(QString uniqueKeyField READ uniqueKeyField WRITE setUniqueKeyField NOTIFY uniqueKeyFieldChanged)
    Q_PROPERTY(QString listProperty READ listProperty WRITE setListProperty NOTIFY listPropertyChanged)
    Q_PROPERTY(qint64 errorCode READ errorCode NOTIFY errorChanged)
    Q_PROPERTY(QVariant errorValue READ errorValue NOTIFY errorChanged)
    Q_PROPERTY(bool refreshing READ refreshing NOTIFY lastRequestChanged)

public:

    enum SortMode {
        SortNoSort,
        SortAscending,
        SortDescending
    };

    AsemanGeneralModel(QObject *parent = Q_NULLPTR);
    virtual ~AsemanGeneralModel();

    Q_INVOKABLE QStringList roles() const;

    QString itemId(const QModelIndex &index) const;

    qint32 rowCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, qint32 role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, qint32 role = Qt::DisplayRole);

    QHash<qint32, QByteArray> roleNames() const;
    qint32 count() const;

    void setAgent(AsemanAbstractAgentClient *agent);
    AsemanAbstractAgentClient *agent() const;

    void setMethod(const QString &method);
    QString method() const;

    void setArguments(const QVariantList &arguments);
    QVariantList arguments() const;

    void setUniqueKeyField(const QString &uniqueKeyField);
    QString uniqueKeyField() const;

    void setListProperty(const QString &listProperty);
    QString listProperty() const;

    void setSortMode(qint32 sortMode);
    qint32 sortMode() const;

    qint32 offset() const;

    qint32 limit() const;
    void setLimit(qint32 limit);

    bool refreshing() const;

    qint64 errorCode() const;
    QVariant errorValue() const;

public Q_SLOTS:
    void clear();
    void refresh();
    void more();

    QVariant get(qint32 index, qint32 role) const;
    QVariant get(qint32 index, const QString &roleName) const;
    QVariantMap get(qint32 index) const;

Q_SIGNALS:
    void countChanged();
    void agentChanged();
    void methodChanged();
    void argumentsChanged();
    void offsetChanged();
    void uniqueKeyFieldChanged();
    void listPropertyChanged();
    void sortModeChanged();
    void limitChanged();
    void lastRequestChanged();
    void listChanged();
    void error(qint32 errorCode, const QVariant &errorValue);
    void errorChanged();

private:
    void processAnswer(qint64 itemId, const QVariant &result);
    void processError(qint64 itemId, qint32 errorCode, const QVariant &errorValue);

protected:
    void refresh_prv();
    void setOffset(qint32 offset);
    void setLastRequest(qint64 lastRequest);
    void changed(const QStringList &list);

private:
    Private *p;
};

#endif // ASEMANGENERALMODEL_H
