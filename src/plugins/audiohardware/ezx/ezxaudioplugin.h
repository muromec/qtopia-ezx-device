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

#ifndef __EZX_AUDIO_PLUGIN_H__
#define __EZX_AUDIO_PLUGIN_H__

#include <QAudioStatePlugin>

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <alsa/asoundlib.h>


class EZXAudioPluginPrivate;

class EZXAudioPlugin : public QAudioStatePlugin
{
    Q_OBJECT
    friend class EZXAudioPluginPrivate;

public:
    EZXAudioPlugin(QObject *parent = 0);
    ~EZXAudioPlugin();

    QList<QAudioState *> statesProvided() const;

private:
    EZXAudioPluginPrivate *m_data;

    int initMixer;
    int closeMixer;
    void select_item(char *elem_name, char *item);


};

#endif
