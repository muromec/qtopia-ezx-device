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

#ifndef EZXMODEMSIMINFO_H
#define EZXMODEMSIMINFO_H

#include <qsiminfo.h>
#include <qmodemsiminfo.h>

class QModemService;
class QAtResult;
class EzxModemSimInfoPrivate;

class QTOPIAPHONEMODEM_EXPORT EzxModemSimInfo : public QModemSimInfo
{
    Q_OBJECT
public:
    explicit EzxModemSimInfo( QModemService *service );
    ~EzxModemSimInfo();

private slots:
    void cimi( bool ok, const QAtResult& result );

private:
    EzxModemSimInfoPrivate *d;

};

#endif /* EZXMODEMSIMINFO_H */
