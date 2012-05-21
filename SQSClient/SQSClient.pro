HEADERS += \
    SQSClient.h

SOURCES += \
    SQSClient.cpp \
    Main.cpp

unix:!macx:!symbian: LIBS += -L$$PWD/../libevent-2.0.19/lib/ -levent

INCLUDEPATH += $$PWD/../libevent-2.0.19/include
DEPENDPATH += $$PWD/../libevent-2.0.19/include
