QT       += core gui widgets network serialport printsupport xml script qml
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Добавляем флаги для поддержки исключений и отладки
QMAKE_CXXFLAGS += -fexceptions
CONFIG += debug

QMAKE_LFLAGS += -static-libgcc -static-libstdc++

# Для Windows
win32 {
    CONFIG += windows
    LIBS += -lsetupapi
}

# Добавляем директории для поиска заголовочных файлов
INCLUDEPATH += $$PWD

SOURCES += \
    main.cpp \
    configmanager.cpp \
    expression.cpp \
    j2534.cpp \
    loggerdefinitionloader.cpp \
    loggerwindow.cpp \
    parameter.cpp \
    parametergraphicswidget.cpp \
    parameterselectiondialog.cpp \
    parameterwidget.cpp \
    parameterwindow.cpp \
    preferencesdialog.cpp \
    qcustomplot.cpp

HEADERS += \
    configmanager.h \
    expression.h \
    expressionparser.h \
    j2534.h \
    j2534_tactrix.h \
    loggerdefinitionloader.h \
    loggerwindow.h \
    parameter.h \
    parameterconfig.h \
    parameterdefinition.h \
    parametergraphicswidget.h \
    parameterselectiondialog.h \
    parameterwidget.h \
    parameterwindow.h \
    preferencesdialog.h \
    qcustomplot.h \
    settings.h

FORMS += \
    loggerwindow.ui

# Определяем макросы для отладки
CONFIG(debug, debug|release) {
    DEFINES += DEBUG
    DEFINES += QT_DEBUG
}

CONFIG(release, debug|release) {
    DEFINES += QT_NO_DEBUG_OUTPUT
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
