TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    definition.cc \
    production.cc \
    random.cc

HEADERS += \
    definition.h \
    production.h \
    random.h
