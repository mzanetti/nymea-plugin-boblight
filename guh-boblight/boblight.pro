include(plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginboblight)

message("Building $$deviceplugin$${TARGET}.so")

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


