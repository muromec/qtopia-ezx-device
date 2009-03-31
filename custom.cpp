/****************************************************************************
**
** (c) 2008. Ilya Petrov <ilya.muromec@gmail.com> 
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <custom.h>
#include <qtopianamespace.h>
#include <QtopiaServiceRequest>
#include <QtopiaIpcEnvelope>

#include <qwindowsystem_qws.h>
#include <QValueSpaceItem>
#include <QValueSpaceObject>
#include <stdio.h>
#include <stdlib.h>
#include <QProcess>
#include <QFile>
#include <QFileInfo>

#include <QTextStream>
#include <QDebug>

#define BL "/sys/class/backlight/pwm-backlight/"

QTOPIABASE_EXPORT int qpe_sysBrightnessSteps()
{
    QFile maxBrightness;
    QString strvalue;
    maxBrightness.setFileName(BL "max_brightness");

    if(!maxBrightness.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning()<<"Bl file not opened";
    } else {
        QTextStream in(&maxBrightness);
        in >> strvalue;
        maxBrightness.close();
    }
    return  strvalue.toInt();
}

QTOPIABASE_EXPORT void qpe_setBrightness(int b)
{
    char cmd[80];

    int brightessSteps = qpe_sysBrightnessSteps();
    if(b > brightessSteps)
        b = brightessSteps;

    if(b == 1) {
        // dim
        b = brightessSteps / 4;
    } else if (b == -1) {
        //bright
        b = brightessSteps;
    }

    QFile brightness;
    brightness.setFileName(BL "brightness");

    if(!brightness.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning()<<"Bl File not opened";
    } else {
        QTextStream out(&brightness);
        out << QString::number(b);
        brightness.close();
    }
}

