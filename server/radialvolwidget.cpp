#include <QPaintEvent>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QRadialGradient>
#include <QFont>
#include <QTimer>

#include <QDebug>
#include <qtopialog.h>

#include "radialvolwidget.h"

struct RadialVolumeWidgetPrivate
{
  QRadialGradient spectrum;
  QTimer idle_tmr;
  QPixmap bg;
  int level;
  bool hidden;
  RadialVolumeWidgetPrivate(RadialVolumeWidget *owner);
  void paintBackground();
};

static const float   levels[]        = {0, 0.4, 0.55, 0.70, 0.85, 1.0};
static const int   n_levels          = sizeof(levels)/sizeof(float);
static const int    values[n_levels] = {0,  20,  40,  60,  80, 100};
static const QColor colors[n_levels] =
{
  QColor( 15,   100,  0, 255),
  QColor( 40,   255,  0, 255),
  QColor(192,   255,  0, 255),
  QColor(222,   130,  0, 210),
  QColor(255,     0,  0, 150),
  QColor(255,     0,  0, 100)
};

RadialVolumeWidgetPrivate::RadialVolumeWidgetPrivate(RadialVolumeWidget *owner)
  : idle_tmr(owner), hidden(true)
{
  for (int i=0; i<n_levels; i++)
    spectrum.setColorAt(levels[i], colors[i].darker(150));

  idle_tmr.setSingleShot(true);
  idle_tmr.setInterval(3000);
  owner->connect(&idle_tmr, SIGNAL(timeout()), SLOT(hideControl()));
}

void RadialVolumeWidgetPrivate::paintBackground() // QGradient is soooo sloooow
{
  QPainter p(&bg);
  p.setRenderHint(QPainter::Antialiasing, true);

  int r = spectrum.radius();
  QPoint c = spectrum.center().toPoint();

  p.setBrush(spectrum);
  p.setPen(QPen(Qt::NoPen));
  p.drawEllipse(c.x()-r, c.y()-r, 2*r, 2*r); // Gradient background

  p.setBrush(QBrush()); // Do not fill
  //p.setPen(QPen(QColor(255, 255, 255, 160), 0, Qt::DashLine));
  for (int i=0; i<n_levels; i++)
  {
    int l_r = r*levels[i];
    p.setPen(QPen(QColor(255, 255, 255, 255-(180*(i+1))/n_levels), 1, Qt::DashLine));
    p.drawEllipse(c.x()-l_r, c.y()-l_r, 2*l_r, 2*l_r);
  }
}

//-------------------

RadialVolumeWidget::RadialVolumeWidget(QWidget *parent)
  : QWidget(parent), d(new RadialVolumeWidgetPrivate(this))
{
  qLog() << "RadialVolumeWidget";

  d->level = 0;
}

RadialVolumeWidget::~RadialVolumeWidget()
{
}

void RadialVolumeWidget::stepDown()
{
  qLog() << "RadialVolumeWidget::stepDown()";
  showControl();
  if (d->level>0)
    d->level--;
  update();
}

void RadialVolumeWidget::stepUp()
{
  qLog() << "RadialVolumeWidget::stepUp()";
  showControl();
  if (d->level<n_levels-1)
    d->level++;
  update();
}

void RadialVolumeWidget::paintEvent(QPaintEvent *)
{
  if (d->hidden)
    return;

  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing, true);

  int r = d->spectrum.radius();
  QPoint c(width()/2, height()/2);
  p.drawPixmap(c.x()-r, c.y()-r, d->bg);

  p.setBrush(QColor(255, 255, 255, 100));
  p.setPen(QPen(Qt::NoPen));
  int cl_r = r*levels[d->level];
  p.drawEllipse(c.x()-cl_r, c.y()-cl_r, 2*cl_r, 2*cl_r);
}

void RadialVolumeWidget::resizeEvent(QResizeEvent *)
{
  int radius = qMax(width(), height())*0.47;
  d->bg = QPixmap(radius*2, radius*2);
  d->bg.fill(QColor(0, 0, 0, 0));
  d->spectrum.setCenter(radius, radius);
  d->spectrum.setFocalPoint(radius, radius);
  d->spectrum.setRadius(radius);
  d->paintBackground();
}

void RadialVolumeWidget::showControl()
{
  d->hidden = false;
  update();
  d->idle_tmr.start();
}

void RadialVolumeWidget::hideControl()
{
  d->hidden = true;
  update();
}

