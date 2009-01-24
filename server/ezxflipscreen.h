#ifndef _EZXFLIPSCREEN_H_
#define _EZXFLIPSCREEN_H_

#include <QWidget>
#include <ThemedView>

class EzxFlipScreen: public ThemedView
{
  Q_OBJECT
  public:
    EzxFlipScreen(QWidget *parent = 0);
    virtual ~EzxFlipScreen();

    QWidget *newWidget(ThemeWidgetItem *item, const QString &name);
  signals:
    void volume_stepUp();
    void volume_stepDown();
  public slots:
    void volumeUp();
    void volumeDown();
    void showMenu();
};

#endif // _EZXFLIPSCREEN_H_
