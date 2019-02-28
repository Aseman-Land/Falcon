#ifndef ASEMANCLIENTQML_H
#define ASEMANCLIENTQML_H

#include <QString>

class AsemanClientQml
{
public:
#ifdef ASEMAN_STATIC_BUILD
    static bool registerTypes();
#endif
    static void registerTypes(const QString &uri);

private:
#ifdef ASEMAN_STATIC_BUILD
    static bool static_types_registered;
#endif
};

#endif // ASEMANCLIENTQML_H
