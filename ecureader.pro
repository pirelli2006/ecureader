QT += core gui widgets network serialport qml printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    configmanager.cpp \
    j2534.cpp \
    loggerdefinitionloader.cpp \
    loggerwindow.cpp \
    main.cpp \
    mainwindow.cpp \
    parameter.cpp \
    parameterselectiondialog.cpp \
    parameterwidget.cpp \
    parameterwindow.cpp \
    preferencesdialog.cpp \
    qcustomplot.cpp

HEADERS += \
    configmanager.h \
    j2534.h \
    j2534_tactrix.h \
    loggerdefinitionloader.h \
    loggerwindow.h \
    mainwindow.h \
    parameter.h \
    parameterconfig.h \
    parameterdefinition.h \
    parameterselectiondialog.h \
    parameterwidget.h \
    parameterwindow.h \
    preferencesdialog.h \
    qcustomplot.h

FORMS += \
    loggerwindow.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
