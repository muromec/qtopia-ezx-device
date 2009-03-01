#ifndef _FLIPVOLWIDGET_H_
#define _FLIPVOLWIDGET_H_

#include <QWidget>
#include <QTimeLine>

class AudioVolumeManager;
struct RadialVolumeWidgetPrivate;

class RadialVolumeWidget: public QWidget
{
  Q_OBJECT
  public:
    RadialVolumeWidget(QWidget *parent = NULL);
    virtual ~RadialVolumeWidget();
  signals:
    void volumeChanged(int volume);
  public slots:
    void stepUp();
    void stepDown();

    void showControl();
    void hideControl();
  protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
  private:
    RadialVolumeWidgetPrivate *d;
    AudioVolumeManager *manager;
};

#endif // _FLIPVOLWIDGET_H_
