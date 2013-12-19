#-------------------------------------------------
#
# Project created by QtCreator 2013-11-24T23:22:34
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

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
    INCLUDEPATH += irrKlang-1.4.0/include
    LIBS += D:\dev\multiplayermuisc\MultiplayerMusic-client\irrKlang-1.4.0\lib\Win32-visualStudio/irrKlang.lib
}
unix {
   LIBS += -lX11
}

FORMS    += mainwindow.ui
