TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
    hashset.c \
    streamtokenizer.c \
    vector.c

HEADERS += \
    bool.h \
    hashset.h \
    streamtokenizer.h \
    vector.h
