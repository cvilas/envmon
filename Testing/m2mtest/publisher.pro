TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += publisher_main.cpp \
    ../../../../theirs/mosquitto-1.2.3/lib/cpp/mosquittopp.cpp \
    publisher.cpp


unix:!macx: LIBS += -L$$PWD/../../../../theirs/mosquitto-1.2.3/lib/ -lmosquitto

INCLUDEPATH += $$PWD/../../../../theirs/mosquitto-1.2.3 \
                $$PWD/../../../../theirs/mosquitto-1.2.3/lib\
                 $$PWD/../../../../theirs/mosquitto-1.2.3/lib/cpp
DEPENDPATH += $$PWD/../../../../theirs/mosquitto-1.2.3

unix:!macx: PRE_TARGETDEPS += $$PWD/../../../../theirs/mosquitto-1.2.3/lib/libmosquitto.a

HEADERS += \
    ../../../../theirs/mosquitto-1.2.3/lib/cpp/mosquittopp.h \
    publisher.h
