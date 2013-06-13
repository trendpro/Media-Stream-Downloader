#-------------------------------------------------
#
# Project created by QtCreator 2012-10-30T16:16:19
#
#-------------------------------------------------

QT       += core gui
QT       += network

TARGET = Stream_Downloader
TEMPLATE = app


SOURCES += main.cpp\
    mainwindow.cpp \
    downloadworker.cpp \
    downloaddialog.cpp \
    src/qrtmp.cpp \
    src/librtmp/rtmp.c \
    src/librtmp/parseurl.c \
    src/librtmp/log.cpp \
    src/librtmp/hashswf.c \
    src/librtmp/amf.c

HEADERS  += mainwindow.h \
    downloadworker.h \
    downloaddialog.h \
    src/qrtmp.h \
    src/librtmp/rtmp_sys.h \
    src/librtmp/rtmp.h \
    src/librtmp/log.h \
    src/librtmp/http.h \
    src/librtmp/handshake.h \
    src/librtmp/dhgroups.h \
    src/librtmp/dh.h \
    src/librtmp/bytes.h \
    src/librtmp/amf.h

FORMS    += mainwindow.ui

unix:LIBS += -lssl -lcrypto
win32:LIBS += libwsock32
win32:LIBS += libwinmm

INCLUDEPATH = -L"$$_PRO_FILE_PWD_/Library/openssl-1.0.0/include" -L"$$_PRO_FILE_PWD_/Library/zlib/include"
DEPENDPATH = --L"$$_PRO_FILE_PWD_/Library/openssl-1.0.0/include" -L"$$_PRO_FILE_PWD_/Library/zlib/include"

win32:LIBS += Library/zlib/lib/libz.dll.a

win32:LIBS += Library/openssl-1.0.0/lib/libcrypto.dll.a
win32:LIBS += Library/openssl-1.0.0/lib/libssl.dll.a
