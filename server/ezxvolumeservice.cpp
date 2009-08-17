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

void EzxVolumeService::adjustSpeakerVolume( int value )
{

  initMixer();

  for ( elem = snd_mixer_first_elem( mixerFd); elem; elem = snd_mixer_elem_next( elem) ) {
      if ( snd_mixer_elem_get_type( elem ) == SND_MIXER_ELEM_SIMPLE &&
           snd_mixer_selem_is_active( elem) ) {

          elemName = QString(snd_mixer_selem_get_name( elem));

          if(elemName == "Master") { // Master output // could use PCM
              if(snd_mixer_selem_has_playback_volume( elem) > 0)
                snd_mixer_selem_set_playback_volume_all(elem, (long)(value*15/100));

              break;
          }
      }
  }

  closeMixer();
}

void EzxVolumeService::adjustVolume(int leftChannel, int rightChannel, AdjustType adjust)
{
      int leftright, left, right;

      if (adjust == Relative) {
          leftright = m_d->currVolume;

          left = leftright;
          right = leftright;

          left = qBound(0, left + leftChannel, 100);
          right = qBound(0, right + rightChannel, 100);

      } else {
          left = leftChannel;
          right = rightChannel;
      }

      m_d->currVolume = (left + right) >> 1;
      adjustSpeakerVolume( m_d->currVolume );

}

int EzxVolumeService::initMixer()
{
    int result;
  if ((result = snd_mixer_open( &mixerFd, 0)) < 0) {
        qWarning()<<"snd_mixer_open error"<< result;
        mixerFd = NULL;
        return result;
    }
/*  hw:0
  hw:0,0*/
    if ((result = snd_mixer_attach( mixerFd, "default")) < 0) {
        qWarning()<<"snd_mixer_attach error"<< result;
        snd_mixer_close(mixerFd);
        mixerFd = NULL;
        return result;
    }
    if ((result = snd_mixer_selem_register( mixerFd, NULL, NULL)) < 0) {
        qWarning()<<"snd_mixer_selem_register error"<<result;
        snd_mixer_close(mixerFd);
        mixerFd = NULL;
        return result;
    }
    if ((result = snd_mixer_load( mixerFd)) < 0) {
        qWarning()<<"snd_mixer_load error"<<result;
        snd_mixer_close(mixerFd);
        mixerFd = NULL;
        return result;
    }
    return result;
}

int EzxVolumeService::closeMixer()
{
     int result = snd_mixer_detach( mixerFd, "default" );
     result = snd_mixer_close( mixerFd );
//     snd_mixer_free( mixerFd ); //causes segfault
    return 0;

}


QTOPIA_TASK(EzxVolumeService, EzxVolumeService);

