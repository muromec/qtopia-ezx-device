qtopia_project(qtopia plugin)
TARGET=ezxaudiohardware

HEADERS		=  ezxaudioplugin.h
SOURCES	        =  ezxaudioplugin.cpp

depends(libraries/qtopiaaudio)
enable_bluetooth {
    depends(libraries/qtopiacomm)
}

LIBS += -lasound
