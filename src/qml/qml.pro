TARGET  = falconqml
TARGETPATH = Falcon
IMPORT_VERSION = 1.0
VERSION_REVISION = 1

INCLUDEPATH += $$PWD
QT += core gui network falcon qml quick

HEADERS += \
    $$PWD/asemanclientqml_plugin.h \
    $$PWD/asemanclientqml.h \
    $$PWD/services/auth1.h \
    $$PWD/services/file1.h

SOURCES += \
    $$PWD/asemanclientqml_plugin.cpp \
    $$PWD/asemanclientqml.cpp

static: !linux: !win32: DEFINES += ASEMAN_STATIC_BUILD

load(qml_plugin)

RESOURCES += \
    falconqml_resource.qrc
