#-------------------------------------------------
#
# Project created by QtCreator 2013-11-24T23:22:34
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia

TARGET = MultiplayerMusic-client
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    netstate.cpp \
    soundmanager.cpp \
    console.cpp

HEADERS  += mainwindow.h \
    netstate.h \
    soundmanager.h \
    console.h

win32 {
}
unix {
   LIBS += -lX11
}

FORMS    += mainwindow.ui
