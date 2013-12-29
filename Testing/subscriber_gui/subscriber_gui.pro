#-------------------------------------------------
#
# Project created by QtCreator 2013-12-29T18:53:44
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = subscriber_gui
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ../../ThirdParty/mosquitto/src/mosquittopp.cpp \
    ../m2mtest/subscriber.cpp


HEADERS  += mainwindow.h \
    ../../ThirdParty/mosquitto/include/mosquitto/mosquittopp.h \
    ../../ThirdParty/mosquitto/include/mosquitto/mosquitto.h \
    ../m2mtest/subscriber.h

FORMS    += mainwindow.ui

CONFIG += mobility
MOBILITY = 

unix:!android: LIBS += -L$$PWD/../../ThirdParty/mosquitto/lib/linux64/
android: LIBS += -L$$PWD/../../ThirdParty/mosquitto/lib/android/
LIBS += -lmosquitto

INCLUDEPATH += $$PWD/../../ThirdParty/mosquitto/include\
                $$PWD/../../ThirdParty/mosquitto/include/mosquitto\
                $$PWD/../../Testing/m2mtest

android: PRE_TARGETDEPS += $$PWD/../../ThirdParty/mosquitto/lib/android/libmosquitto.a
unix:!android: PRE_TARGETDEPS += $$PWD/../../ThirdParty/mosquitto/lib/linux64/libmosquitto.a
