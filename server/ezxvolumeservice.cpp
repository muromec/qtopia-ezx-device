/****************************************************************************
**
** This file is part of the Qtopia Opensource Edition Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** versions 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/soundcard.h>

#include <QTimer>

#include <qtopiaserverapplication.h>
#include <qtopiaipcenvelope.h>

#include "ezxvolumeservice.h"

class EzxVolumeServicePrivate 
{
public:
    
    void sendCurrentVolume()
    {
        QString volume;
        volume.setNum(currVolume);
        QtopiaIpcEnvelope e("QPE/AudioVolumeManager","currentVolume(QString)");
        e << volume;
    };

    int currVolume;
};

EzxVolumeService::EzxVolumeService():
    QtopiaIpcAdaptor("QPE/AudioVolumeManager/EzxVolumeService")
{
    publishAll(Slots);

    m_d = new EzxVolumeServicePrivate;
    m_d->currVolume = 0;
    setVolume(60);

    QTimer::singleShot(0, this, SLOT(registerService()));
}

EzxVolumeService::~EzxVolumeService()
{
    delete m_d;
}

//public slots:
void EzxVolumeService::setVolume(int volume)
{
    adjustVolume(volume, volume, Absolute);
}

void EzxVolumeService::setVolume(int leftChannel, int rightChannel)
{
    adjustVolume(leftChannel, rightChannel, Absolute);
}

void EzxVolumeService::increaseVolume(int increment)
{
    adjustVolume(increment, increment, Relative);
    m_d->sendCurrentVolume();
}

void EzxVolumeService::decreaseVolume(int decrement)
{
    decrement *= -1;

    adjustVolume(decrement, decrement, Relative);
    m_d->sendCurrentVolume();

}

void EzxVolumeService::setMute(bool)
{
}

void EzxVolumeService::registerService()
{
    QtopiaIpcEnvelope   e("QPE/AudioVolumeManager", "registerHandler(QString,QString)");

    e << QString("Headset") << QString("QPE/AudioVolumeManager/EzxVolumeService");

    QTimer::singleShot(0, this, SLOT(setCallDomain()));
}

void EzxVolumeService::setCallDomain()
{
    QtopiaIpcEnvelope   e("QPE/AudioVolumeManager", "setActiveDomain(QString)");

    e << QString("Headset");
}


void EzxVolumeService::adjustVolume(int leftChannel, int rightChannel, AdjustType adjust)
{
    int mixerFd = open("/dev/mixer", O_RDWR);
    if (mixerFd >= 0) {
        int leftright, left, right;

        if (adjust == Relative) {
            ioctl(mixerFd, MIXER_READ(SOUND_MIXER_OGAIN), &leftright);

            left = leftright; 
            right = leftright; 

            left = qBound(0, left + leftChannel, 100);
            right = qBound(0, right + rightChannel, 100);

        } else {
            left = leftChannel;
            right = rightChannel;
        }

        leftright = (left + right) >> 1;
        int input = leftright;
        m_d->currVolume = leftright;
        ioctl(mixerFd, MIXER_WRITE(SOUND_MIXER_OGAIN), &leftright);       
        ioctl(mixerFd, MIXER_WRITE(SOUND_MIXER_IGAIN), &input);
        close(mixerFd);
    }
}

QTOPIA_TASK(EzxVolumeService, EzxVolumeService);

