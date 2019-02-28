#include "asemangeneralmodel.h"

#include <QHash>
#include <QPointer>
#include <QTimer>

class AsemanGeneralModel::Private
{
public:
    QStringList list;
    QHash<QString, QVariantMap> hash;

    QHash<qint32, QByteArray> roleNames;
    QSet<QString> roleKeys;

    QPointer<AsemanAbstractAgentClient> agent;
    QString method;
    QVariantList arguments;
    QString uniqueKeyField;
    QString listProperty;

    qint32 offset;
    qint32 limit;
    qint32 sortMode;

    qint64 lastRequest;
    QTimer *refreshTimer;

    qint64 errorCode;
    QVariant errorValue;
};

AsemanGeneralModel::AsemanGeneralModel(QObject *parent) :
    QAbstractListModel(parent)
{
    p = new Private;
    p->offset = 0;
    p->limit = 100;
    p->errorCode = false;
    p->lastRequest = -1;
    p->uniqueKeyField = QStringLiteral("id");
    p->sortMode = SortNoSort;

    p->refreshTimer = new QTimer(this);
    p->refreshTimer->setInterval(200);
    p->refreshTimer->setSingleShot(true);

    connect(p->refreshTimer, &QTimer::timeout, this, &AsemanGeneralModel::refresh_prv);
}

QStringList AsemanGeneralModel::roles() const
{
    QStringList result;
    const QHash<qint32,QByteArray> &roles = roleNames();
    QHashIterator<qint32,QByteArray> i(roles);
    while(i.hasNext())
    {
        i.next();
        result << QString::fromUtf8(i.value());
    }

    std::sort(result.begin(), result.end());
    return result;
}

QString AsemanGeneralModel::itemId(const QModelIndex &index) const
{
    qint32 row = index.row();
    if(row >= count())
        return QString();
    else
        return p->list.at(row);
}

qint32 AsemanGeneralModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return count();
}

QVariant AsemanGeneralModel::data(const QModelIndex &index, qint32 role) const
{
    QString id = itemId(index);
    if(id.isEmpty())
        return QVariant();

    QVariantMap map = p->hash.value(id);
    if(map.isEmpty())
        return QVariant();

    QString roleName = QString::fromUtf8(p->roleNames.value(role));
    return map.value(roleName);
}

bool AsemanGeneralModel::setData(const QModelIndex &index, const QVariant &value, qint32 role)
{
    QString id = itemId(index);
    if(id.isEmpty())
        return false;

    QString roleName = QString::fromUtf8(p->roleNames.value(role));
    if(roleName.isEmpty())
        return false;

    p->hash[id][roleName] = value;
    return true;
}

QHash<qint32, QByteArray> AsemanGeneralModel::roleNames() const
{
    return p->roleNames;
}

qint32 AsemanGeneralModel::count() const
{
    return p->list.count();
}

void AsemanGeneralModel::setAgent(AsemanAbstractAgentClient *agent)
{
    if(p->agent == agent)
        return;

    if(p->agent)
    {
        disconnect(p->agent, &AsemanAbstractAgentClient::answer, this, &AsemanGeneralModel::processAnswer);
        disconnect(p->agent, &AsemanAbstractAgentClient::error, this, &AsemanGeneralModel::processError);
    }

    p->agent = agent;
    if(p->agent)
    {
        connect(p->agent, &AsemanAbstractAgentClient::answer, this, &AsemanGeneralModel::processAnswer);
        connect(p->agent, &AsemanAbstractAgentClient::error, this, &AsemanGeneralModel::processError);
    }

    refresh();
    Q_EMIT agentChanged();
}

AsemanAbstractAgentClient *AsemanGeneralModel::agent() const
{
    return p->agent;
}

void AsemanGeneralModel::setMethod(const QString &method)
{
    if(p->method == method)
        return;

    p->method = method;
    refresh();
    Q_EMIT methodChanged();
}

QString AsemanGeneralModel::method() const
{
    return p->method;
}

void AsemanGeneralModel::setArguments(const QVariantList &arguments)
{
    if(p->arguments == arguments)
        return;

    p->arguments = arguments;
    refresh();
    Q_EMIT argumentsChanged();
}

QVariantList AsemanGeneralModel::arguments() const
{
    return p->arguments;
}

void AsemanGeneralModel::setUniqueKeyField(const QString &uniqueKeyField)
{
    if(p->uniqueKeyField == uniqueKeyField)
        return;

    p->uniqueKeyField = uniqueKeyField;
    clear();
    refresh();
    Q_EMIT uniqueKeyFieldChanged();
}

QString AsemanGeneralModel::uniqueKeyField() const
{
    return p->uniqueKeyField;
}

void AsemanGeneralModel::setListProperty(const QString &listProperty)
{
    if(p->listProperty == listProperty)
        return;

    p->listProperty = listProperty;
    clear();
    refresh();
    Q_EMIT listPropertyChanged();
}

QString AsemanGeneralModel::listProperty() const
{
    return p->listProperty;
}

void AsemanGeneralModel::setSortMode(qint32 sortMode)
{
    if(p->sortMode == sortMode)
        return;

    p->sortMode = sortMode;
    if(p->sortMode == SortNoSort)
        clear();
    refresh();
    Q_EMIT sortModeChanged();
}

qint32 AsemanGeneralModel::sortMode() const
{
    return p->sortMode;
}

qint32 AsemanGeneralModel::offset() const
{
    return p->offset;
}

void AsemanGeneralModel::setOffset(qint32 offset)
{
    if(p->offset == offset)
        return;

    p->offset = offset;
    refresh();
    Q_EMIT offsetChanged();
}

void AsemanGeneralModel::changed(const QStringList &list)
{
    bool count_changed = (list.count()!=p->list.count());

    for( int i=0 ; i<p->list.count() ; i++ )
    {
        const QString &file = p->list.at(i);
        if( list.contains(file) )
            continue;

        beginRemoveRows(QModelIndex(), i, i);
        p->list.removeAt(i);
        i--;
        endRemoveRows();
    }

    QList<QString> temp_list = list;
    for( int i=0 ; i<temp_list.count() ; i++ )
    {
        const QString &file = temp_list.at(i);
        if( p->list.contains(file) )
            continue;

        temp_list.removeAt(i);
        i--;
    }
    while( p->list != temp_list )
        for( int i=0 ; i<p->list.count() ; i++ )
        {
            const QString &file = p->list.at(i);
            int nw = temp_list.indexOf(file);
            if( i == nw )
                continue;

            beginMoveRows( QModelIndex(), i, i, QModelIndex(), nw>i?nw+1:nw );
            p->list.move( i, nw );
            endMoveRows();
        }

    for( int i=0 ; i<list.count() ; i++ )
    {
        const QString &file = list.at(i);
        if( p->list.contains(file) )
            continue;

        beginInsertRows(QModelIndex(), i, i );
        p->list.insert( i, file );
        endInsertRows();
    }

    if(count_changed)
        Q_EMIT countChanged();

    Q_EMIT listChanged();
}

void AsemanGeneralModel::processAnswer(qint64 id, const QVariant &_result)
{
    if(id != p->lastRequest)
        return;

    setLastRequest(-1);

    QVariant result = _result;
    if(p->listProperty.count())
        result = _result.toMap().value(p->listProperty);

    if(result.type() != QVariant::List)
        return;

    QStringList res = p->list;
    QVariantList list = result.toList();

    bool keyIsNum = true;
    for(const QVariant &l: list)
    {
        if(l.type() != QVariant::Map)
            continue;

        QVariantMap map = l.toMap();
        if(!map.contains(p->uniqueKeyField))
            continue;

        QStringList keys = map.keys();
        for(const QString &k: keys)
        {
            if(p->roleKeys.contains(k))
                continue;

            p->roleKeys.insert(k);
            p->roleNames[ Qt::UserRole + 1000 + p->roleKeys.count() ] = k.toUtf8();
        }

        QVariant uniqueKeyVar = map.value(p->uniqueKeyField);
        bool isDouble = (uniqueKeyVar.type() == QVariant::Double);
        if(uniqueKeyVar.type() != QVariant::String && !uniqueKeyVar.convert(QVariant::String))
            continue;

        QString uniqueKey = uniqueKeyVar.toString();

        bool isNum = isDouble;
        if(!isDouble)
            uniqueKey.toInt(&isNum);
        keyIsNum = keyIsNum & isNum;

        p->hash[uniqueKey] = map;

        if(uniqueKey.isEmpty() || res.contains(uniqueKey))
            continue;

        res << uniqueKey;
    }

    if(keyIsNum && p->sortMode != SortNoSort)
    {
        QList<qint64> numsRes;
        for(const QString &r: res)
            numsRes << r.toLongLong();

        switch( static_cast<qint32>(p->sortMode) )
        {
        case SortAscending:
            std::sort(numsRes.begin(), numsRes.end());
            break;
        case SortDescending:
            std::sort(numsRes.begin(), numsRes.end(), std::greater<int>());
            break;
        }

        res.clear();
        for(qint64 n: numsRes)
            res << QString::number(n);
    }

    changed(res);
}

void AsemanGeneralModel::processError(qint64 id, qint32 errorCode, const QVariant &errorValue)
{
    if(id != p->lastRequest)
        return;

    setLastRequest(-1);

    p->errorCode = errorCode;
    p->errorValue = errorValue;

    Q_EMIT error(errorCode, errorValue);
    Q_EMIT errorChanged();
}

qint32 AsemanGeneralModel::limit() const
{
    return p->limit;
}

void AsemanGeneralModel::setLimit(qint32 limit)
{
    if(p->limit == limit)
        return;

    p->limit = limit;
    refresh();
    Q_EMIT limitChanged();
}

bool AsemanGeneralModel::refreshing() const
{
    return p->lastRequest != -1;
}

void AsemanGeneralModel::setLastRequest(qint64 lastRequest)
{
    if(p->lastRequest == lastRequest)
        return;

    p->lastRequest = lastRequest;
    Q_EMIT lastRequestChanged();
}

qint64 AsemanGeneralModel::errorCode() const
{
    return p->errorCode;
}

QVariant AsemanGeneralModel::errorValue() const
{
    return p->errorValue;
}

void AsemanGeneralModel::clear()
{
    changed( QStringList() );
    setOffset(0);
    p->hash.clear();
    p->roleNames.clear();
    p->roleKeys.clear();
}

void AsemanGeneralModel::refresh()
{
    p->refreshTimer->stop();
    p->refreshTimer->start();
}

void AsemanGeneralModel::more()
{
    if(p->lastRequest != -1)
        return;

    setOffset(p->list.count());
}

void AsemanGeneralModel::refresh_prv()
{
    if(!p->agent)
        return;

    p->errorCode = 0;
    p->errorValue.clear();

    qint64 lastRequest = p->agent->pushRequest(p->method, p->arguments, 1000, true);
    setLastRequest(lastRequest);

    Q_EMIT errorChanged();
}

QVariant AsemanGeneralModel::get(qint32 row, qint32 role) const
{
    if(row >= rowCount() || row < 0)
        return QVariant();

    const QModelIndex &idx = index(row,0);
    return data(idx , role);
}

QVariant AsemanGeneralModel::get(qint32 index, const QString &roleName) const
{
    const qint32 role = roleNames().key(roleName.toUtf8());
    return get(index, role);
}

QVariantMap AsemanGeneralModel::get(qint32 index) const
{
    if(index >= rowCount())
        return QVariantMap();

    QVariantMap result;
    const QHash<qint32,QByteArray> &roles = roleNames();
    QHashIterator<qint32,QByteArray> i(roles);
    while(i.hasNext())
    {
        i.next();
        result[QString::fromUtf8(i.value())] = get(index, i.key());
    }

    return result;
}

AsemanGeneralModel::~AsemanGeneralModel()
{
    delete p;
}
