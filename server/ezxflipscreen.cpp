#include "ezxflipscreen.h"
#include "radialvolwidget.h"

#include <qtopialog.h>
#include "themecontrol.h"

#include <QtopiaIpcEnvelope>

EzxFlipScreen::EzxFlipScreen(QWidget *parent)
  : ThemedView(parent)
{
  ThemeControl::instance()->registerThemedView(this, "ClosedFlipScreen");

  ipc = new QtopiaChannel("QPE/FlipScreen", this);
  connect(
    ipc,  SIGNAL(received(QString,QByteArray)),
    this, SLOT(ipcEvent(QString,QByteArray))
  );

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
    connect(this, SIGNAL(volume_stepUp()), vol, SLOT(stepUp()));
    connect(this, SIGNAL(volume_stepDown()), vol, SLOT(stepDown()));
    return vol;
  }
  return NULL;
}

void EzxFlipScreen::volumeUp()
{
  qLog() << "EzxFlipScreen::volumeUp()";
  emit volume_stepUp();
}

void EzxFlipScreen::volumeDown()
{
  qLog() << "EzxFlipScreen::volumeDown()";
  emit volume_stepDown();
}

void EzxFlipScreen::showMenu()
{
}

void EzxFlipScreen::ipcEvent(const QString &msg, const QByteArray &arg) 
{
  if (msg == "hide()") {
    qLog(Hardware) << "hide flip";
    hide();
  }
}
