# -------------------------------------------------
# Project created by QtCreator 2009-03-07T14:54:40
# -------------------------------------------------
QT += network \
    xml
TARGET = qrabbit
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    hubconnection.cpp \
    hubnicklist.cpp \
    tablemodel.cpp \
    filemanager.cpp \
    tth/tiger_data.c \
    tth/tiger.c \
    tth/hashfile.cpp \
    tth/hash.cpp \
    tth/base32.cpp \
    initfiletree.cpp \
    searchmanager.cpp \
    hubtcpsocket.cpp \
    hubudpsocket.cpp \
    clientconnection.cpp \
    clienttcpsocket.cpp
HEADERS += mainwindow.h \
    hubconnection.h \
    global.h \
    hubnicklist.h \
    tablemodel.h \
    filemanager.h \
    tth/tiger.h \
    tth/hashfile.h \
    tth/hash.h \
    tth/byte_order.h \
    tth/base32.h \
    fileinfo.h \
    dirstree.h \
    initfiletree.h \
    searchmanager.h \
    searchitem.h \
    hubtcpsocket.h \
    hubudpsocket.h \
    clientconnection.h \
    clienttcpsocket.h
FORMS += mainwindow.ui
LIBS += -lbz2
