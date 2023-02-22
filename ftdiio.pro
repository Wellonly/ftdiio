#-------------------------------------------------
#
# Project created by QtCreator 2018-01-10T19:40:26
#
#-------------------------------------------------

QT       += core gui charts widgets
qtHaveModule(printsupport): QT += printsupport

CONFIG += qt c++14 static

TARGET = timerio
TEMPLATE = app

DEFINES += NO_EXCEPTIONS NO_LIB

#FIXME: need this libs from ftdi.com: LIBS += -LLIB/ftdi/i386 -lftd2xx

INCLUDEPATH += LIB
INCLUDEPATH += LIB/ftdi

HEADERS += \
    mainwidget.h \
    LIB/ftdi/ftd2xx.h \
    LIB/datetime.h \
    LIB/zvvlib.h \
    LIB/zvvdebug.h \
    LIB/jconfig.h \
    LIB/ftdi/ftdilib.h

#    LIB/time.h

SOURCES += \
    main.cpp \
    LIB/zvvlib.cpp \
    LIB/jconfig.cpp \
    mainwidget.cpp \
    LIB/ftdi/ftdilib.cpp

#    LIB/time.cpp

RESOURCES += \
    ftdiio.qrc

OTHER_FILES += "dist.bat"


