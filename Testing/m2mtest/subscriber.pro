TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += subscriber_main.cpp \
    ../../ThirdParty/mosquitto/src/mosquittopp.cpp \
    subscriber.cpp


LIBS += -L$$PWD/../../ThirdParty/mosquitto/lib/linux64/ -lmosquitto -lpthread -lrt

INCLUDEPATH += ../../ThirdParty/mosquitto/include/mosquitto/ \
                ../../station/

HEADERS += \
    ../../ThirdParty/mosquitto/include/mosquitto/mosquittopp.h \
    ../../ThirdParty/mosquitto/include/mosquitto/mosquitto.h \
    subscriber.h
