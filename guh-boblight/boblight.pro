include(/usr/include/nymea/plugins.pri)

QT += dbus bluetooth

CONFIG += c++11

TARGET = $$qtLibraryTarget(nymea_devicepluginboblight)

INCLUDEPATH += /usr/local/include/
LIBS += -L/usr/local/lib/ -lboblight

SOURCES += \
    devicepluginboblight.cpp \
    bobclient.cpp \
    bobchannel.cpp

HEADERS += \
    devicepluginboblight.h \
    bobclient.h \
    bobchannel.h


