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

#ifndef __QTOPIA_MEDIA_DEFAULTVOLUMEPROVIDER_H
#define __QTOPIA_MEDIA_DEFAULTVOLUMEPROVIDER_H
#include <alsa/asoundlib.h>

#include <qtopiaipcadaptor.h>

class EzxVolumeServicePrivate;

class EzxVolumeService : public QtopiaIpcAdaptor
{
    Q_OBJECT
    enum AdjustType { Relative, Absolute };

public:
    EzxVolumeService();
    ~EzxVolumeService();

public slots:
    void setVolume(int volume);
    void setVolume(int leftChannel, int rightChannel);
    void increaseVolume(int increment);
    void decreaseVolume(int decrement);
    void setMute(bool mute);

private slots:
    void registerService();
    void setCallDomain();

private:
    void adjustVolume(int leftChannel, int rightChannel, AdjustType);
    void adjustSpeakerVolume(int value);

    int initMixer();
    int closeMixer();

    snd_mixer_t *mixerFd;
    snd_mixer_elem_t *elem;
    QString elemName;

    int m_leftChannelVolume;
    int m_rightChannelVolume;

    EzxVolumeServicePrivate *m_d;
};


#endif  // __QTOPIA_MEDIA_DEFAULTVOLUMEPROVIDER_H
