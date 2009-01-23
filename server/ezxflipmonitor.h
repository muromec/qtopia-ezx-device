#ifndef _EZXFLIPMONITOR_H_
#define _EZXFLIPMONITOR_H_

#include <QObject>
#include <qtopiainputevents.h>
#include <QValueSpaceItem>

class EzxFlipScreen;

class EzxFlipMonitor: public QObject, public QtopiaKeyboardFilter
{
  Q_OBJECT
  public:
    EzxFlipMonitor(QObject *parent = NULL);
    virtual ~EzxFlipMonitor();
    bool filter(int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat);
  private slots:
    void updateFlipScreenState();
  private:
    EzxFlipScreen *scr;
    QValueSpaceItem clamshellVsi;
};

#endif // _EZXFLIPMONITOR_H_
