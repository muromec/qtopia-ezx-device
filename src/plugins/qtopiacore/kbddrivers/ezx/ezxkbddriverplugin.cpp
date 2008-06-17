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

#include "ezxkbddriverplugin.h"
#include "ezxkbdhandler.h"

#include <qtopiaglobal.h>
#include <qtopialog.h>

EZXKbdDriverPlugin::EZXKbdDriverPlugin( QObject *parent )
    : QKbdDriverPlugin( parent )
{}

EZXKbdDriverPlugin::~EZXKbdDriverPlugin()
{}

QWSKeyboardHandler* EZXKbdDriverPlugin::create(const QString& driver, const QString&)
{
    qLog(Input) << "EZXKbdDriverPlugin:create()";
    return create( driver );
}

QWSKeyboardHandler* EZXKbdDriverPlugin::create( const QString& driver)
{
    if( driver.toLower() == "ezxkbdhandler" ) {
        qLog(Input) << "Before call EZXKbdHandler()";
        return new EZXKbdHandler();
    }
    return 0;
}

QStringList EZXKbdDriverPlugin::keys() const
{
    return QStringList() << "ezxkbdhandler";
}

QTOPIA_EXPORT_QT_PLUGIN(EZXKbdDriverPlugin)
