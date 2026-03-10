QT       += core gui sql printsupport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# QCustomPlot is large, this flag can help with "file too big" errors
QMAKE_CXXFLAGS += -Wa,-mbig-obj

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    home.cpp \
    databasemanager.cpp \
    loginwidget.cpp \
    dashboardwidget.cpp \
    qcustomplot.cpp \
    tcpmanager.cpp \
    devicecontrolwidget.cpp \
    historywidget.cpp \
    scenewidget.cpp \
    aiassistant.cpp \
    aiassistantwidget.cpp \
    deviceitemwidget.cpp \
    devicemanager.cpp \
    sceneeditdialog.cpp

HEADERS += \
    home.h \
    databasemanager.h \
    loginwidget.h \
    dashboardwidget.h \
    qcustomplot.h \
    tcpmanager.h \
    devicecontrolwidget.h \
    historywidget.h \
    scenewidget.h \
    aiassistant.h \
    aiassistantwidget.h \
    deviceitemwidget.h \
    devicemanager.h \
    scenemodels.h \
    sceneeditdialog.h

FORMS += \
    home.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
