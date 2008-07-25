qtopia_project(qtopia plugin)
TARGET=ezxvendor

CONFIG+=no_tr

HEADERS		=  ezxplugin.h   ezxbattery.h   ezxmodemservice.h \
  ezxvibrateaccessory.h \
  ezxmodempinmanager.h \
  ezxmodemnetworkregistration.h \
  ezxmodemsiminfo.h \
  ezxmodemsupplementaryservices.h \
  ezxmodemsmssender.h
SOURCES	        =  ezxplugin.cpp ezxbattery.cpp ezxmodemservice.cpp \
  ezxvibrateaccessory.cpp \
  ezxmodempinmanager.cpp \
  ezxmodemnetworkregistration.cpp \
  ezxmodemsiminfo.cpp \
  ezxmodemsupplementaryservices.cpp \
  ezxmodemsmssender.cpp


depends(libraries/qtopiaphonemodem)

