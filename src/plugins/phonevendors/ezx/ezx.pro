qtopia_project(qtopia plugin)
TARGET=ezxvendor

CONFIG+=no_tr
LIBS += -lezxtapi

HEADERS		=  ezxplugin.h   phoneservertapimodem.h   tapiezxbattery.h
SOURCES	        =  ezxplugin.cpp phoneservertapimodem.cpp tapiezxbattery.cpp

depends(libraries/qtopiaphonemodem)

