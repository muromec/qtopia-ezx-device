qtopia_project(qtopia plugin)
TARGET=ezxmultiplex

CONFIG+=no_tr

HEADERS		=  ezxmultiplexer.h
SOURCES	        =  ezxmultiplexer.cpp

requires(enable_modem)
depends(libraries/qtopiacomm/serial)
