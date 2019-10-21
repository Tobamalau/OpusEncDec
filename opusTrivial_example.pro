TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    mainEnc.c

HEADERS += \
    ../../../../usr/local/include/opus/opus.h \
    ../../../../usr/local/include/opus/opus_defines.h \
    ../../../../usr/local/include/opus/opus_multistream.h \
    ../../../../usr/local/include/opus/opus_projection.h \
    ../../../../usr/local/include/opus/opus_types.h

INCLUDEPATH += ../../../../usr/local/include/opus/
LIBS += -L../../../../usr/local/lib/ -lopus

DISTFILES += \
    MarioTest.wav
