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

#include "ezxaudioplugin.h"

#include <QAudioState>
#include <QAudioStateInfo>
#include <QValueSpaceItem>
#include <QtopiaIpcAdaptor>
#ifdef QTOPIA_BLUETOOTH
#include <QBluetoothAudioGateway>
#endif

#include <QDebug>
#include <qplugin.h>
#include <qtopialog.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <dirent.h>
#include <stdio.h>

#include <string.h>

#include <qaudiooutput.h>
#include "asoc_bp.h"

#define ENABLE(x) set_state(x, 1); 
#define DISABLE(x) set_state(x, 0);
#define DISABLE_ALL() set_state(0, 0);

#define _I "Input Mixer "
#define _O "Output Mixer "
  
snd_mixer_t *mixerFd;

int initMixer()
{
  int result;
  if ((result = snd_mixer_open( &mixerFd, 0)) < 0) {
        printf("sucks1!\n");
        mixerFd = NULL;
        return result;
    }
    
    if ((result = snd_mixer_attach( mixerFd, "default")) < 0) {
        printf("snd_mixer_attach error");
        snd_mixer_close(mixerFd);
        mixerFd = NULL;
        return result;
    }
    if ((result = snd_mixer_selem_register( mixerFd, NULL, NULL)) < 0) {
        printf("snd_mixer_selem_register error");
        snd_mixer_close(mixerFd);
        mixerFd = NULL;
        return result;
    }
    if ((result = snd_mixer_load( mixerFd)) < 0) {
        printf("snd_mixer_load error");
        snd_mixer_close(mixerFd);
        mixerFd = NULL;
        return result;
    }
    return result;
}
int closeMixer()
{
     int result = snd_mixer_detach( mixerFd, "default" );
     result = snd_mixer_close( mixerFd );
    return 0;
 
}

void set_state(char *elem_name, int state)
{
  snd_mixer_elem_t *elem; 
  for ( elem = snd_mixer_first_elem( mixerFd); elem; elem = snd_mixer_elem_next( elem) ) {

      printf("want %s, found %s\n",
          elem_name, 
          snd_mixer_selem_get_name( elem)

      );

      if (elem_name && strcmp(elem_name,snd_mixer_selem_get_name( elem)))
        continue;

      snd_mixer_selem_set_playback_switch_all(elem, state);


  }


}


#ifdef QTOPIA_BLUETOOTH
class BluetoothAudioState : public QAudioState
{
    Q_OBJECT
public:
    explicit BluetoothAudioState(bool isPhone, QObject *parent = 0);
    virtual ~BluetoothAudioState();

    virtual QAudioStateInfo info() const;
    virtual QAudio::AudioCapabilities capabilities() const;

    virtual bool isAvailable() const;
    virtual bool enter(QAudio::AudioCapability capability);
    virtual bool leave();

private slots:
    void bluetoothAudioStateChanged();
    void headsetDisconnected();
    void headsetConnected();

private:
    bool resetCurrAudioGateway();

private:
    QList<QBluetoothAudioGateway *> m_audioGateways;
    bool m_isPhone;
    QBluetoothAudioGateway *m_currAudioGateway;
    QtopiaIpcAdaptor *adaptor;
    QAudioStateInfo m_info;
    bool m_isActive;
    bool m_isAvail;
};

BluetoothAudioState::BluetoothAudioState(bool isPhone, QObject *parent)
    : QAudioState(parent)
{
    m_isPhone = isPhone;
    m_currAudioGateway = 0;
    m_isActive = false;

    QBluetoothAudioGateway *hf = new QBluetoothAudioGateway("BluetoothHandsfree");
    m_audioGateways.append(hf);
    qLog(AudioState) << "Handsfree audio gateway: " << hf;

    QBluetoothAudioGateway *hs = new QBluetoothAudioGateway("BluetoothHeadset");
    m_audioGateways.append(hs);
    qLog(AudioState) << "Headset audio gateway: " << hs;

    for (int i=0; i<m_audioGateways.size(); i++) {
        QBluetoothAudioGateway *gateway = m_audioGateways.at(i);
        connect(gateway, SIGNAL(audioStateChanged()), SLOT(bluetoothAudioStateChanged()));
        connect(gateway, SIGNAL(headsetDisconnected()), SLOT(headsetDisconnected()));
        connect(gateway, SIGNAL(connectResult(bool,QString)),
                SLOT(headsetConnected()));
        connect(gateway, SIGNAL(newConnection(QBluetoothAddress)),
                SLOT(headsetConnected()));
    }

    if (isPhone) {
        m_info.setDomain("Phone");
        m_info.setProfile("PhoneBluetoothHeadset");
        m_info.setPriority(25);
    } else {
        m_info.setDomain("Media");
        m_info.setProfile("MediaBluetoothHeadset");
        m_info.setPriority(150);
    }

    m_info.setDisplayName(tr("Bluetooth Headset"));

    m_isAvail = false;
    if(resetCurrAudioGateway())
        m_isAvail = true;
}

BluetoothAudioState::~BluetoothAudioState()
{
    for (int i = 0; i < m_audioGateways.size(); i++) {
        delete m_audioGateways.at(i);
    }
}

bool BluetoothAudioState::resetCurrAudioGateway()
{
    for (int i=0; i<m_audioGateways.size(); i++) {
        QBluetoothAudioGateway *gateway = m_audioGateways.at(i);
        if (gateway->isConnected()) {
            m_currAudioGateway = gateway;
            qLog(AudioState) << "Returning audiogateway to be:" << m_currAudioGateway;
            return true;
        }
    }

    qLog(AudioState) << "No current audio gateway found";
    return false;
}

void BluetoothAudioState::bluetoothAudioStateChanged()
{
    qLog(AudioState) << "bluetoothAudioStateChanged" << m_isActive << m_currAudioGateway;

    if (m_isActive && (m_currAudioGateway || resetCurrAudioGateway())) {
        if (!m_currAudioGateway->audioEnabled()) {
            emit doNotUseHint();
        }
    }
    else if (!m_isActive && (m_currAudioGateway || resetCurrAudioGateway())) {
        if (m_currAudioGateway->audioEnabled()) {
            emit useHint();
        }
    }
}

void BluetoothAudioState::headsetConnected()
{
    if (!m_isAvail && resetCurrAudioGateway()) {
        m_isAvail = true;
        emit availabilityChanged(true);
    }
}

void BluetoothAudioState::headsetDisconnected()
{
    if (!resetCurrAudioGateway()) {
        m_isAvail = false;
        emit availabilityChanged(false);
    }
}

QAudioStateInfo BluetoothAudioState::info() const
{
    return m_info;
}

QAudio::AudioCapabilities BluetoothAudioState::capabilities() const
{
    // Cannot record Bluetooth convos, check with Aaron M.
    if (m_isPhone) {
        return QAudio::OutputOnly;
    }
    else {
        return QAudio::OutputOnly | QAudio::InputOnly;
    }
}

bool BluetoothAudioState::isAvailable() const
{
    return m_isAvail;
}

bool BluetoothAudioState::enter(QAudio::AudioCapability capability)
{
  return true;
}

bool BluetoothAudioState::leave()
{

    if (m_currAudioGateway || resetCurrAudioGateway()) {
        m_currAudioGateway->releaseAudio();
    }

    m_isActive = false;

    return true;
}
#endif

class SpeakerAudioState : public QAudioState
{
    Q_OBJECT

public:
    SpeakerAudioState(bool isPhone, QObject *parent = 0);

    QAudioStateInfo info() const;
    QAudio::AudioCapabilities capabilities() const;

    bool isAvailable() const;
    bool enter(QAudio::AudioCapability capability);
    bool leave();

private:
    QAudioStateInfo m_info;
    bool m_isPhone;
};

SpeakerAudioState::SpeakerAudioState(bool isPhone, QObject *parent)
    : QAudioState(parent)
{
    m_isPhone = isPhone;

    if (isPhone) {
        m_info.setDomain("Phone");
        m_info.setProfile("PhoneSpeaker");
        m_info.setDisplayName(tr("Handset"));
    } else {
        m_info.setDomain("Media");
        m_info.setProfile("MediaSpeaker");
        m_info.setDisplayName(tr("Stereo Speaker"));
    }

    m_info.setPriority(100);
}

QAudioStateInfo SpeakerAudioState::info() const
{
    return m_info;
}

QAudio::AudioCapabilities SpeakerAudioState::capabilities() const
{
    return QAudio::InputOnly | QAudio::OutputOnly;
}

bool SpeakerAudioState::isAvailable() const
{
    return true;
}

bool SpeakerAudioState::enter(QAudio::AudioCapability capability)
{
  printf("SpeakerAudioState::enter\n");

  initMixer();
  if (m_isPhone) {
    ENABLE(_O"A1");
    ENABLE(_I"A5");
    open_bp();
  } else {
    close_bp();
    ENABLE(_O"A2");
  }

  closeMixer();

  return true;

}

bool SpeakerAudioState::leave()
{
  initMixer();
  if (m_isPhone) {
    DISABLE(_O"A1");
    DISABLE(_I"A5");
  } else {
    DISABLE(_O"A2");
  }

  closeMixer();

  return true;
}

class HeadphonesAudioState : public QAudioState
{
    Q_OBJECT

public:
    HeadphonesAudioState(bool isPhone, QObject *parent = 0);

    QAudioStateInfo info() const;
    QAudio::AudioCapabilities capabilities() const;

    bool isAvailable() const;
    bool enter(QAudio::AudioCapability capability);
    bool leave();

private slots:
    void onHeadsetModified();

private:
    QAudioStateInfo m_info;
    bool m_isPhone;
    QValueSpaceItem *m_headset;
    QtopiaIpcAdaptor *adaptor;
    int headType;
    
};

HeadphonesAudioState::HeadphonesAudioState(bool isPhone, QObject *parent)
    : QAudioState(parent)
{
    m_isPhone = isPhone;

    if (isPhone) {
        m_info.setDomain("Phone");
        m_info.setProfile("PhoneHeadphones");
        m_info.setDisplayName(tr("Headphones"));
    } else {
        m_info.setDomain("Media");
        m_info.setProfile("MediaHeadphones");
        m_info.setDisplayName(tr("Headphones"));
    }

    m_info.setPriority(50);

    m_headset = new QValueSpaceItem("/Hardware/Accessories/PortableHandsfree/Present", this);
    connect( m_headset, SIGNAL(contentsChanged()),
             this, SLOT(onHeadsetModified()));
    onHeadsetModified();

}

QAudioStateInfo HeadphonesAudioState::info() const
{
    return m_info;
}

QAudio::AudioCapabilities HeadphonesAudioState::capabilities() const
{
    return QAudio::InputOnly | QAudio::OutputOnly;
}

void HeadphonesAudioState::onHeadsetModified()
{
    bool avail = m_headset->value(false).toBool();
    headType = m_headset->value(0).toInt();

    emit availabilityChanged(avail);
}

bool HeadphonesAudioState::isAvailable() const
{
    return m_headset->value(false).toBool();
}

bool HeadphonesAudioState::enter(QAudio::AudioCapability capability)
{
    Q_UNUSED(capability)

    initMixer();

    ENABLE(_O"AR");
    ENABLE(_O"AL");

    closeMixer();
    return true;


}

bool HeadphonesAudioState::leave()
{
    initMixer();

    DISABLE(_O"AR");
    DISABLE(_O"AL");

    closeMixer();

    return true;
}

class SpeakerphoneAudioState : public QAudioState
{
    Q_OBJECT

public:
    SpeakerphoneAudioState(QObject *parent = 0);

    QAudioStateInfo info() const;
    QAudio::AudioCapabilities capabilities() const;

    bool isAvailable() const;
    bool enter(QAudio::AudioCapability capability);
    bool leave();

private:
    QAudioStateInfo m_info;
    
};

SpeakerphoneAudioState::SpeakerphoneAudioState(QObject *parent)
    : QAudioState(parent)
{

    m_info.setDomain("Phone");
    m_info.setProfile("PhoneSpeakerphone");
    m_info.setDisplayName(tr("Speakerphone"));
    m_info.setPriority(200);
}

QAudioStateInfo SpeakerphoneAudioState::info() const
{
    return m_info;
}

QAudio::AudioCapabilities SpeakerphoneAudioState::capabilities() const
{
    return QAudio::InputOnly | QAudio::OutputOnly;
}

bool SpeakerphoneAudioState::isAvailable() const
{
    return true;
}

bool SpeakerphoneAudioState::enter(QAudio::AudioCapability capability)
{
    initMixer();
    ENABLE(_O"A2"); // loudspeaker
    ENABLE(_I"A5"); // built-in mic
    closeMixer();

    return true;
}

bool SpeakerphoneAudioState::leave()
{
    initMixer();
    DISABLE(_O"A2"); // loudspeaker
    DISABLE(_I"A5"); // built-in mic
    closeMixer();

    return true;
}

class RingtoneAudioState : public QAudioState
{
    Q_OBJECT

public:
    RingtoneAudioState(QObject *parent = 0);

    QAudioStateInfo info() const;
    QAudio::AudioCapabilities capabilities() const;

    bool isAvailable() const;
    bool enter(QAudio::AudioCapability capability);
    bool leave();

private:
    QAudioStateInfo m_info;
};

RingtoneAudioState::RingtoneAudioState(QObject *parent)
    : QAudioState(parent)
{
    m_info.setDomain("RingTone");
    m_info.setProfile("RingToneSpeaker");
    m_info.setDisplayName(tr("Stereo"));
    m_info.setPriority(100);
}

QAudioStateInfo RingtoneAudioState::info() const
{
    return m_info;
}

QAudio::AudioCapabilities RingtoneAudioState::capabilities() const
{
    return QAudio::OutputOnly;
}

bool RingtoneAudioState::isAvailable() const
{
    return true;
}

bool RingtoneAudioState::enter(QAudio::AudioCapability)
{
  printf("ringtone enter\n");
    return true;
}

bool RingtoneAudioState::leave()
{
  printf("ringtone leave\n");
    return true;
}

class EZXAudioPluginPrivate
{
public:
    QList<QAudioState *> m_states;
};

EZXAudioPlugin::EZXAudioPlugin(QObject *parent)
    : QAudioStatePlugin(parent)
{
    m_data = new EZXAudioPluginPrivate;

    m_data->m_states.push_back(new SpeakerAudioState(false, this));
    m_data->m_states.push_back(new SpeakerAudioState(true, this));

    m_data->m_states.push_back(new HeadphonesAudioState(false, this));
    m_data->m_states.push_back(new HeadphonesAudioState(true, this));

#ifdef QTOPIA_BLUETOOTH
    // Can play media through bluetooth.  Can record through bluetooth as well.
    m_data->m_states.push_back(new BluetoothAudioState(false, this));
    m_data->m_states.push_back(new BluetoothAudioState(true, this));
#endif

    m_data->m_states.push_back(new SpeakerphoneAudioState(this));

    m_data->m_states.push_back(new RingtoneAudioState(this));

    //TODO: Need to enable Bluetooth RingTone
}

EZXAudioPlugin::~EZXAudioPlugin()
{
    for (int i = 0; m_data->m_states.size(); i++) {
        delete m_data->m_states.at(i);
    }

    delete m_data;
}

QList<QAudioState *> EZXAudioPlugin::statesProvided() const
{
    return m_data->m_states;
}

Q_EXPORT_PLUGIN2(ezxaudio_plugin, EZXAudioPlugin)

#include "ezxaudioplugin.moc"
