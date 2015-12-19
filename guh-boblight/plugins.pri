TEMPLATE = lib
CONFIG += plugin

QT += network

QMAKE_CXXFLAGS += -Werror
CONFIG += c++11

INCLUDEPATH += /usr/include/guh
LIBS += -lguh

infofile.output = plugininfo.h
infofile.commands = /usr/bin/guh-generateplugininfo ${QMAKE_FILE_NAME} ${QMAKE_FILE_OUT}
infofile.depends = /usr/bin/guh-generateplugininfo
infofile.CONFIG = no_link
JSONFILES = deviceplugin"$$TARGET".json
infofile.input = JSONFILES

QMAKE_EXTRA_COMPILERS += infofile

target.path = /usr/lib/guh/plugins/
INSTALLS += target
