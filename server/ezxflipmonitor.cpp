#include <qtopialog.h>
#include <windowmanagement.h>
#include <qtopiaserverapplication.h>

#include "ezxflipscreen.h"
#include "ezxflipmonitor.h"

QTOPIA_TASK(EzxFlipMonitor, EzxFlipMonitor);

EzxFlipMonitor::EzxFlipMonitor(QObject *parent)
  : QObject(parent),
  scr(new EzxFlipScreen()),
  clamshellVsi("/Hardware/Devices/ClamshellOpen", this),
  keyLockVso("/UI", this) // Override builtin keylock to tel Qtopia not to intercept key events
{
  QtopiaInputEvents::addKeyboardFilter(this);
  connect(&clamshellVsi, SIGNAL(contentsChanged()), SLOT(updateFlipScreenState()));
}

EzxFlipMonitor::~EzxFlipMonitor()
{
}

bool EzxFlipMonitor::filter(int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat)
{
  if (keycode==Qt::Key_Flip) // do not block self
    return false;
  if (!clamshellVsi.value().toBool()) // flip is closed
  {
    qLog() << "EzxFlipMonitor: Kbd event captured" << keycode;
    if (isPress)
    {
      switch (keycode)
      {
        case Qt::Key_VolumeUp:
        case Qt::Key_F30:
          scr->volumeUp();
          break;
        case Qt::Key_VolumeDown:
        case Qt::Key_F31:
          scr->volumeDown();
          break;
      }
    }
    return true;
  }
  return false;
}

void EzxFlipMonitor::updateFlipScreenState()
{
  bool on = clamshellVsi.value().toBool();
  if (on)
  {
    qLog() << "EzxFlipMonitor: Flip opened";
    scr->hide();
    keyLockVso.setAttribute("KeyLock", false);
  }
  else
  {
    qLog() << "EzxFlipMonitor: Flip closed";
    scr->showFullScreen();
    WindowManagement::protectWindow(scr);
    keyLockVso.setAttribute("KeyLock", true);
  }
}
