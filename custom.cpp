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

#define BL "/sys/class/backlight/pwm-backlight.0/"

QFile *value = NULL;
QFile *power = NULL;

#define OPEN_QFILE(x,s,m) { \
    x = new QFile(); \
    x->setFileName(BL s); \
    ret = x->open(m); \
    if (!ret) { \
      qWarning()<<"cant open" << s; \
      delete x; \
      x = NULL; \
    } \
}

#define WO (QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)
#define RO (QIODevice::ReadOnly | QIODevice::Text)


QTOPIABASE_EXPORT int qpe_sysBrightnessSteps()
{
    QFile *maxBrightness;
    QString strvalue;
    int ret;

    OPEN_QFILE(value, "brightness", WO);
    OPEN_QFILE(power, "bl_power", WO);
    OPEN_QFILE(maxBrightness, "max_brightness", RO);

    if(maxBrightness) {
      QTextStream in(maxBrightness);
      in >> strvalue;
      maxBrightness->close();
      delete maxBrightness;
    }
    return  strvalue.toInt();
}

QTOPIABASE_EXPORT void qpe_setBrightness(int b)
{

    if (! (value && power) )
      return;

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

    if(value) {
        QTextStream out(value);
        out << QString::number(b);
    }

    if(power) {
        QTextStream out(power);

        if (b) {
          qWarning() << "lcd power on";
          out << "0";
        } else {
          qWarning() << "lcd power off";
          out << "1";
        }
    }
}

