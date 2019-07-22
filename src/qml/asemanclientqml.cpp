#include "asemanclientqml.h"
#include "services/file1.h"

#include <asemanremotefile.h>
#include <asemangeneralmodel.h>
#include <asemanclientsocket.h>
#include <qqml.h>

#ifdef ASEMAN_STATIC_BUILD
bool AsemanClientQml::static_types_registered = AsemanClientQml::registerTypes();

bool AsemanClientQml::registerTypes()
{
    if(static_types_registered)
        return true;

    registerTypes("Falcon.Base");
    return true;
}
#endif

void AsemanClientQml::registerTypes(const QString &uri)
{
    qmlRegisterType<File1>(uri.toUtf8(), 1, 0, "File");
    qmlRegisterType<AsemanRemoteFile>(uri.toUtf8(), 1, 0, "RemoteFile");
    qmlRegisterType<AsemanGeneralModel>(uri.toUtf8(), 1, 0, "GeneralModel");
    qmlRegisterType<AsemanClientSocket>(uri.toUtf8(), 1, 0, "ClientSocket");
    qmlRegisterType(QUrl(QLatin1String("qrc:/Falcon/qml/RemoteImage.qml")), uri.toUtf8(), 1, 0, "RemoteImage");
}
