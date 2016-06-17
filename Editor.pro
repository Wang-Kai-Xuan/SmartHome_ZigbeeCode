TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += UART.C \
    74LS164_8LED.c \
    OSAL_SD_App.c \
    SD_App.c \
    SD_AppHw.c

HEADERS += \
    74LS164_8LED.h \
    SD_App.h \
    SD_AppHw.h \
    UART.H
