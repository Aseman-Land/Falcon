load(qt_build_config)

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

TARGET = AsemanFalcon
CONFIG += c++11
QT += network gui qml

MODULE = falcon

load(qt_module)

DEFINES += ASEMANCLIENTLIB_LIBRARY
ios {
    QMAKE_CXXFLAGS += -fvisibility=hidden
}

SOURCES += \
    $$PWD/asemanabstractagentclient.cpp \
    $$PWD/asemanabstractclientsocket.cpp \
    $$PWD/asemaniodevice.cpp \
    $$PWD/asemangeneralmodel.cpp \
    $$PWD/asemanremotefile.cpp \
    $$PWD/asemanclientsocketcontroller.cpp \
    $$PWD/asemanclientsocket.cpp \
    $$PWD/asemantrustsolverengine.cpp


HEADERS +=\
    $$PWD/asemanclientlib_global.h \
    $$PWD/asemanabstractagentclient.h \
    $$PWD/asemanabstractclientsocket.h \
    $$PWD/asemanclientlib.h \
    $$PWD/asemaniodevice.h \
    $$PWD/asemangeneralmodel.h \
    $$PWD/asemanremotefile.h \
    $$PWD/services/file1.h \
    $$PWD/asemanclientsocketcontroller.h \
    $$PWD/asemanclientmacros.h \
    $$PWD/asemanclientsocket.h \
    $$PWD/asemantrustsolverengine.h
