#include <qtopialog.h>
#include <windowmanagement.h>
#include <qtopiaserverapplication.h>

#include "ezxflipscreen.h"
#include "ezxflipmonitor.h"

QTOPIA_TASK(EzxFlipMonitor, EzxFlipMonitor);

EzxFlipMonitor::EzxFlipMonitor(QObject *parent)
  : QObject(parent),
  scr(new EzxFlipScreen()),
  clamshellVsi("/Hardware/Devices/ClamshellOpen", this)
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
  }
  else
  {
    qLog() << "EzxFlipMonitor: Flip closed";
    scr->showFullScreen();
    WindowManagement::protectWindow(scr);
  }
}
