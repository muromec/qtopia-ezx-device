qtopia_project(qtopia plugin)
TARGET=ezxvendor

CONFIG+=no_tr
LIBS += -lezxtapi

HEADERS		=  ezxplugin.h  ezxbattery.h \
 qnetworkregistrationtapi.h \
 qphonebooktapi.h \
 qphonecallprovidertapi.h \
 qphonecalltapi.h \
 qphonerffunctionalitytapi.h \
 qpinmanagertapi.h \
 qsiminfotapi.h \
 qsupplementaryservicestapi.h \
 qtelephonyservicetapi.h \
 ezxvibrateaccessory.h \
 qsmssendertapi.h
SOURCES	        =  ezxplugin.cpp phoneservertapimodem.cpp ezxbattery.cpp \
  qphonecallprovidertapi.cpp \
  qphonebooktapi.cpp \
  qnetworkregistrationtapi.cpp \
  qsiminfotapi.cpp \
  qphonerffunctionalitytapi.cpp \
  qpinmanagertapi.cpp \
  ezxvibrateaccessory.cpp \
  qsupplementaryservicestapi.cpp \
  qsmssendertapi.cpp


depends(libraries/qtopiaphonemodem)

