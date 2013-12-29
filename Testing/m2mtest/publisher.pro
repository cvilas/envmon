TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += publisher_main.cpp \
    ../../ThirdParty/mosquitto/src/mosquittopp.cpp \
    publisher.cpp


LIBS += -L$$PWD/../../ThirdParty/mosquitto/lib/linux64/ -lmosquitto

INCLUDEPATH += ../../ThirdParty/mosquitto/include/mosquitto/

HEADERS += \
    ../../ThirdParty/mosquitto/include/mosquitto/mosquittopp.h \
    ../../ThirdParty/mosquitto/include/mosquitto/mosquitto.h \
    publisher.h
