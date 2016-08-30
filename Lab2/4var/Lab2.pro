QT += core
QT -= gui

TARGET = Lab2
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app


deployment.path = $$OUT_PWD/
deployment.files += input.txt \
                    grammar.txt

INSTALLS += deployment

SOURCES += \
    main.cpp \
    syntaxanalizator.cpp

HEADERS += \
    syntaxanalizator.h

