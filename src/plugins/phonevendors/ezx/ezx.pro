qtopia_project(qtopia plugin)
TARGET=ezxvendor

CONFIG+=no_tr
LIBS += -lezxtapi

HEADERS		=  ezxplugin.h  tapiezxbattery.h \
 qnetworkregistrationtapi.h \
 qphonebooktapi.h \
 qphonecallprovidertapi.h \
 qphonecalltapi.h \
 qphonerffunctionalitytapi.h \
 qpinmanagertapi.h \
 qsiminfotapi.h \
 qsupplementaryservicestapi.h \
 qtelephonyservicetapi.h \
 ezxvibrateaccessory.h
SOURCES	        =  ezxplugin.cpp phoneservertapimodem.cpp tapiezxbattery.cpp

depends(libraries/qtopiaphonemodem)

