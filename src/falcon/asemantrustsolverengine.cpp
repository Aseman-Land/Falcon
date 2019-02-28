/*
    Copyright (C) 2017 Aseman Team
    http://aseman.co

    TelegramStats is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    TelegramStats is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "asemantrustsolverengine.h"
#include "../AsemanGlobals/asemanserverenums.h"

#include <QDataStream>
#include <QIODevice>
#include <QJSEngine>
#include <QDir>
#include <QLibrary>

class AsemanTrustSolverEnginePrivate
{
public:
    QJSEngine *engine;
};

AsemanTrustSolverEngine::AsemanTrustSolverEngine(QObject *parent) : QObject(parent)
{
    p = new AsemanTrustSolverEnginePrivate;
    p->engine = 0;
}

void AsemanTrustSolverEngine::resolve(QByteArray data)
{
    QByteArray result;

    qint64 type = 0;

    QDataStream stream(&data, QIODevice::ReadOnly);
    stream >> type;

    switch(type)
    {
    case AsemanServerEnums::TrustQstTypeBinary:
    {
        QByteArray lib;
        stream >> lib;
        result = solveBinary(lib);
    }
        break;
    case AsemanServerEnums::TrustQstTypeJavaScript:
    {
        QString js;
        stream >> js;
        result = solveJs(js);
    }
        break;
    default:
    case AsemanServerEnums::TrustQstTypeNoNeed:
        result = QByteArray::number(1);
        break;
    }

    Q_EMIT solved(result);
}

QByteArray AsemanTrustSolverEngine::solveJs(const QString &js)
{
    if(!p->engine)
        p->engine = new QJSEngine(this);

    QJSValue fnc = p->engine->evaluate(js);
    QVariant result = fnc.call().toVariant();

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_6);
    stream << result;

    return data;
}

QByteArray AsemanTrustSolverEngine::solveBinary(const QByteArray &libData)
{
    QByteArray data;
    QString path = QDir::homePath() + "/temp";
    QDir().mkpath(path);

    QFile file(path + "/libquestion.so");
    if(!file.open(QFile::WriteOnly))
        return data;

    file.write(libData);
    file.close();

    QLibrary myLib(file.fileName());
    typedef QByteArray (*MyPrototype)();
    MyPrototype myFunction = (MyPrototype) myLib.resolve("startResolving");
    if(myFunction)
        data = myFunction();

    myLib.unload();
    file.remove();
    return data;
}

AsemanTrustSolverEngine::~AsemanTrustSolverEngine()
{
    delete p;
}
