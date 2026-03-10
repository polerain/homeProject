QT       += core gui sql printsupport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# QCustomPlot is large, this flag can help with "file too big" errors
QMAKE_CXXFLAGS += -Wa,-mbig-obj

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    aiassistant.cpp \
    aiassistantwidget.cpp \
    alarmwidget.cpp \
    dashboardwidget.cpp \
    databasemanager.cpp \
    devicecontrolwidget.cpp \
    deviceeditdialog.cpp \
    historywidget.cpp \
    home.cpp \
    loginwidget.cpp \
    main.cpp \
    qcustomplot.cpp \
    sceneeditdialog.cpp \
    sceneexecutor.cpp \
    sceneitemwidget.cpp \
    scenewidget.cpp \
    settingswidget.cpp \
    tcpmanager.cpp \
    weathermanager.cpp

HEADERS += \
    aiassistant.h \
    aiassistantwidget.h \
    alarmwidget.h \
    dashboardwidget.h \
    databasemanager.h \
    devicecontrolwidget.h \
    deviceeditdialog.h \
    historywidget.h \
    home.h \
    loginwidget.h \
    qcustomplot.h \
    sceneeditdialog.h \
    sceneexecutor.h \
    sceneitemwidget.h \
    scenewidget.h \
    settingswidget.h \
    tcpmanager.h \
    weathermanager.h

FORMS += \
    home.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
