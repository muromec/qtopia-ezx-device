/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "ezxmultiplexer.h"

// dummy multyplexer for tapi bridge

QTOPIA_EXPORT_PLUGIN( EZXMultiplexerPlugin )


EZXMultiplexerPlugin::EZXMultiplexerPlugin( QObject* parent )
    : QSerialIODeviceMultiplexerPlugin( parent )
{
}

EZXMultiplexerPlugin::~EZXMultiplexerPlugin()
{
}


bool EZXMultiplexerPlugin::detect( QSerialIODevice *device )
{
    return true;
}

QSerialIODeviceMultiplexer *EZXMultiplexerPlugin::create
        ( QSerialIODevice *device )
{
    return new QNullSerialIODeviceMultiplexer( device );
}
