#ifndef _FLIPVOLWIDGET_H_
#define _FLIPVOLWIDGET_H_

#include <QWidget>
#include <QTimeLine>

struct RadialVolumeWidgetPrivate;

class RadialVolumeWidget: public QWidget
{
  Q_OBJECT
  public:
    RadialVolumeWidget(QWidget *parent = NULL);
    virtual ~RadialVolumeWidget();
  public slots:
    void stepUp();
    void stepDown();
  protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    
    void keyPressEvent(QKeyEvent *ev);
  private:    
    RadialVolumeWidgetPrivate *d;
};

#endif // _FLIPVOLWIDGET_H_
