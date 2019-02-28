#ifndef ASEMANCLIENTQML_PLUGIN_H
#define ASEMANCLIENTQML_PLUGIN_H

#include <QQmlExtensionPlugin>

class AsemanClientQmlPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char *uri);
};

#endif // ASEMANCLIENTQML_PLUGIN_H

