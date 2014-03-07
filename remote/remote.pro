#-------------------------------------------------
#
# Project created by QtCreator 2013-12-29T22:14:13
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = remote
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    indicator.cpp \
    switchstatusindicator.cpp \
    ../ThirdParty/mosquitto/src/mosquittopp.cpp \
    connectionstatusindicator.cpp \
    stationstatusindicator.cpp \
    iotaclient.cpp \
    motionsensestatus.cpp

HEADERS  += mainwindow.h \
    indicator.h \
    switchstatusindicator.h \
    ../station/connection_info.h \
    ../ThirdParty/mosquitto/include/mosquitto/mosquitto.h \
    ../ThirdParty/mosquitto/include/mosquitto/mosquittopp.h \
    connectionstatusindicator.h \
    stationstatusindicator.h \
    ../station/iota_messages.h \
    iotaclient.h \
    motionsensestatus.h

unix:!android:LIBS += -L$$PWD/../ThirdParty/mosquitto/lib/linux64/
android:LIBS += -L$$PWD/../ThirdParty/mosquitto/lib/android/
LIBS += -lmosquitto

INCLUDEPATH += ../ \
                ../ThirdParty/mosquitto/include/mosquitto/ \
                ../station/

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc
