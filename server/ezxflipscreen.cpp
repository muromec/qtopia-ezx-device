#include "ezxflipscreen.h"
#include "radialvolwidget.h"

#include <qtopialog.h>
#include "themecontrol.h"

EzxFlipScreen::EzxFlipScreen(QWidget *parent)
  : ThemedView(parent)
{
  ThemeControl::instance()->registerThemedView(this, "ClosedFlipScreen");
}

EzxFlipScreen::~EzxFlipScreen()
{
}

QWidget *EzxFlipScreen::newWidget(ThemeWidgetItem *item, const QString &name)
{
  // TODO: find a plugin here
  qLog() << "Create widget" << name;
  if (name.toLower()=="volumecontrol")
  {
    RadialVolumeWidget *vol = new RadialVolumeWidget(this);
    connect(this, SIGNAL(volumeUp()), vol, SLOT(stepUp()));
    connect(this, SIGNAL(volumeDown()), vol, SLOT(stepDown()));
    return vol;
  }
  return NULL;
}
