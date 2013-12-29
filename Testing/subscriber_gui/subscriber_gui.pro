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
    ../../ThirdParty/android/src/mosquitto/mosquittopp.cpp \
    ../m2mtest/subscriber.cpp


HEADERS  += mainwindow.h \
    ../../ThirdParty/android/include/mosquitto/mosquittopp.h \
    ../../ThirdParty/android/include/mosquitto/mosquitto.h \
    ../m2mtest/subscriber.h

FORMS    += mainwindow.ui

CONFIG += mobility
MOBILITY = 

unix:!android: LIBS += -L$$PWD/../../../../theirs/mosquitto-1.2.3/lib/
android: LIBS += -L$$PWD/../../ThirdParty/android/lib/
LIBS += -lmosquitto

INCLUDEPATH += $$PWD/../../ThirdParty/android/include\
                $$PWD/../../ThirdParty/android/include/mosquitto\
                $$PWD/../../Testing/m2mtest

DEPENDPATH += $$PWD/../../ThirdParty/android/include

android: PRE_TARGETDEPS += $$PWD/../../ThirdParty/android/lib/libmosquitto.a
unix:!android: PRE_TARGETDEPS += $$PWD/../../../../theirs/mosquitto-1.2.3/lib/libmosquitto.a
