#-------------------------------------------------
#
# Project created by QtCreator 2016-01-18T13:09:03
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PLORE
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    connection.cpp

HEADERS  += widget.h \
    connection.h

FORMS    += widget.ui

OTHER_FILES += \
    update.log
