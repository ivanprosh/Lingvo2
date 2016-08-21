QT += core
QT -= gui

TARGET = Lab1
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    tnode.cpp

HEADERS += \
    tnode.h

deployment.path = $$OUT_PWD/
deployment.files += input.txt \

INSTALLS += deployment

