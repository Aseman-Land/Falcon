#ifndef ASEMANCLIENTLIB_H
#define ASEMANCLIENTLIB_H

#ifdef ASEMAN_FALCON_SERVER
#include "asemanclientsocket.h"
#else
#include "asemanabstractclientsocket.h"
#endif

#ifdef ASEMANCLIENTSOCKET_H
#define ASEMAN_FALCON_QML_INIT qmlRegisterType<AsemanClientSocket>("AsemanServer", 1, 0, "ClientSocket")
#else
#define ASEMAN_FALCON_QML_INIT qmlRegisterType<AsemanAbstractClientSocket>("AsemanServer", 1, 0, "ClientSocket")
#endif

#define ASEMAN_FALCON_QML_REGISTER_TYPE(TYPE, NAME) qmlRegisterType<TYPE>("AsemanServer", 1, 0, NAME)

#endif // ASEMANCLIENTLIB_H
