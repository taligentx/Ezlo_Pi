#-------------------------------------------------
#
# Project created by QtCreator 2022-02-10T16:01:55
#
#-------------------------------------------------

QT       += core gui serialport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ezlogic
TEMPLATE = app

QMAKE_LFLAGS_WINDOWS += /MANIFESTUAC:"level='requireAdministrator'"

SOURCES += main.cpp\
        dialog_config_i2c.cpp \
        dialog_config_input.cpp \
        dialog_config_onewire.cpp \
        dialog_config_spi.cpp \
        dialog_configdev_digitalio.cpp \
        dialog_devadd.cpp \
        dialog_wifi.cpp \
        ezpi_data_types.cpp \
        login.cpp \
        mainwindow.cpp

HEADERS  += mainwindow.h \
    dialog_config_i2c.h \
    dialog_config_input.h \
    dialog_config_onewire.h \
    dialog_config_spi.h \
    dialog_configdev_digitalio.h \
    dialog_devadd.h \
    dialog_wifi.h \
    ezpi_data_types.h \
    ezuuid.h \
    login.h \
    res_strings.h

#CONFIG += console
CONFIG += qt
QT += gui

FORMS    += mainwindow.ui \
    dialog_config_i2c.ui \
    dialog_config_input.ui \
    dialog_config_onewire.ui \
    dialog_config_spi.ui \
    dialog_configdev_digitalio.ui \
    dialog_devadd.ui \
    dialog_wifi.ui \
    login.ui

DISTFILES +=

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES +=
