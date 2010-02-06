qtopia_project(qtopia plugin)
TARGET=ezxaudiohardware

HEADERS		=  ezxaudioplugin.h asoc_bp.h
SOURCES	        =  ezxaudioplugin.cpp asoc_bp.cpp

depends(libraries/qtopiaaudio)
enable_bluetooth {
    depends(libraries/qtopiacomm)
}

LIBS += -lasound -lqtopiaaudio
