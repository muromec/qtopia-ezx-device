/****************************************************************************
**
** This file is part of the Qtopia Opensource Edition Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** versions 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#include <qimage.h>
#include <qpainter.h>

#include "ezxcamera.h"
#include <linux/pxa_camera.h>


#include <QDebug>

#define	VIDEO_DEVICE	    "/dev/video0"


namespace camera
{

bool EzxCamera::hasCamera() const
{
    return ( fd != -1 );
}

QSize EzxCamera::captureSize() const
{
    return QSize( width, height );
}

uint EzxCamera::refocusDelay() const
{
    return 200;
}

int EzxCamera::minimumFramePeriod() const
{
    return (int)(1000/20); // milliseconds
}

EzxCamera::EzxCamera():
    m_imageBuf(0)
{
    setupCamera( QSize( 0, 0 ) );
}

EzxCamera::~EzxCamera()
{
    shutdown();
}

void EzxCamera::setupCamera( QSize size )
{
    qDebug() << "setup" << size.width() << size.height();
    // Clear important variables.
    frames = 0;
    currentFrame = 0;
    caps.minwidth = 144;
    caps.minheight = 176;
    caps.maxwidth = 1024;
    caps.maxheight = 1280;

    // Open the video device.
    fd = open( VIDEO_DEVICE, O_RDWR );
    if ( fd == -1 ) {
        qWarning( "%s: %s", VIDEO_DEVICE, strerror( errno ) );
        return;
    }


    // Determine the capture size to use.  Zero indicates "preview mode".
    if ( size.width() == 0 ) {
        size = recommendedPreviewSize();
    }


    setSize( size );


    // Enable mmap-based access to the camera.
    memset( &mbuf, 0, sizeof( mbuf ) );
    if ( ioctl( fd, VIDIOCGMBUF, &mbuf ) < 0 ) {
        qWarning( "%s: mmap-based camera access is not available", VIDEO_DEVICE );
        close( fd );
        fd = -1;
        return;
    }

    // Mmap the designated memory region.
    frames = (unsigned char *)mmap( 0, mbuf.size, PROT_READ | PROT_WRITE,
				    MAP_SHARED, fd, 0 );
    if ( !frames || frames == (unsigned char *)(long)(-1) ) {
        qWarning( "%s: could not mmap the device", VIDEO_DEVICE );
        close( fd );
        fd = -1;
        return;
    }

   start();
}
void EzxCamera::setSize( QSize size ) {

    height = size.height();
    width  = size.width();

    int ret;

    struct { int r1; int r2; } _pm;

    struct {
        __u16 width, height;
    } csize;

    if (width > 320) {
      _pm.r1 = 20;
      _pm.r2 = 0;
      qDebug() << "photo!";
    } else {
      _pm.r1 = 15;
      _pm.r2 = 15;
    }

    ret = ioctl(fd, WCAM_VIDIOCSINFOR, &_pm);
    qDebug() << "WCAM_VIDIOCSINFOR" << ret;
    ioctl(fd, WCAM_VIDIOCSCINFOR, &_pm);
    qDebug() << "WCAM_VIDIOCSCINFOR" << ret;


    csize.width = width;
    csize.height = height;

    qDebug() << "win" << csize.width << csize.height;;

    /*if ( ioctl( fd, VIDIOCSWIN, &csize ) < 0 ) {
        qWarning("%s: could not set the capture window", VIDEO_DEVICE);
    }*/

    qDebug() << "video" << csize.width << csize.height;;

    if ( ioctl( fd, WCAM_VIDIOCSSSIZE, &csize) < 0) {
        qWarning("cannot set size\n");
    }


    qDebug() << "out" << csize.width << csize.height;;

    if ( ioctl( fd, WCAM_VIDIOCSOSIZE, &csize) < 0) {
        qWarning("cannot set out size\n");
    }


    if ( ioctl( fd, WCAM_VIDIOCSCSIZE, &csize) < 0) {
        qWarning("cannot set still size\n");
    }


    if (m_imageBuf)
      free(m_imageBuf);

    m_imageBuf = (quint16*) malloc(width * height * 2);
}

void EzxCamera::shutdown()
{
    qDebug() << "stop";
    if ( frames != 0 ) {
        qDebug() << "unmap";
        munmap( frames, mbuf.size );
        frames = 0;
    }

    if ( fd != -1 ) {
        stop();
        close( fd );
        fd = -1;
    }

    if (m_imageBuf)
    {
        free(m_imageBuf);
        m_imageBuf = 0;
    }
}

void EzxCamera::stop()
{
   int ret = ioctl(fd, VIDIOCCAPTURE, -1);
   qDebug() << "stop" << ret;
}

void EzxCamera::start() {

  // Set palette mode
    int ret = ioctl(fd, VIDIOCCAPTURE, 0);
  qDebug() << "capture" << ret;
}

void EzxCamera::photo() {

  struct V4l_IMAGE_FRAME frame;
  int nextframe = 1;

  ioctl(fd, VIDIOCCAPTURE, 0);

  ioctl(fd, WCAM_VIDIOCGRABFRAME, &frame);
  ioctl(fd, WCAM_VIDIOCNEXTFRAME, &nextframe);



  int ret = ioctl(fd, VIDIOCCAPTURE, 1);
  qDebug() << "capturing photo" << ret;

}

static const signed short redAdjust[] = {
-161,-160,-159,-158,-157,-156,-155,-153,
-152,-151,-150,-149,-148,-147,-145,-144,
-143,-142,-141,-140,-139,-137,-136,-135,
-134,-133,-132,-131,-129,-128,-127,-126,
-125,-124,-123,-122,-120,-119,-118,-117,
-116,-115,-114,-112,-111,-110,-109,-108,
-107,-106,-104,-103,-102,-101,-100, -99,
 -98, -96, -95, -94, -93, -92, -91, -90,
 -88, -87, -86, -85, -84, -83, -82, -80,
 -79, -78, -77, -76, -75, -74, -72, -71,
 -70, -69, -68, -67, -66, -65, -63, -62,
 -61, -60, -59, -58, -57, -55, -54, -53,
 -52, -51, -50, -49, -47, -46, -45, -44,
 -43, -42, -41, -39, -38, -37, -36, -35,
 -34, -33, -31, -30, -29, -28, -27, -26,
 -25, -23, -22, -21, -20, -19, -18, -17,
 -16, -14, -13, -12, -11, -10,  -9,  -8,
  -6,  -5,  -4,  -3,  -2,  -1,   0,   1,
   2,   3,   4,   5,   6,   7,   9,  10,
  11,  12,  13,  14,  15,  17,  18,  19,
  20,  21,  22,  23,  25,  26,  27,  28,
  29,  30,  31,  33,  34,  35,  36,  37,
  38,  39,  40,  42,  43,  44,  45,  46,
  47,  48,  50,  51,  52,  53,  54,  55,
  56,  58,  59,  60,  61,  62,  63,  64,
  66,  67,  68,  69,  70,  71,  72,  74,
  75,  76,  77,  78,  79,  80,  82,  83,
  84,  85,  86,  87,  88,  90,  91,  92,
  93,  94,  95,  96,  97,  99, 100, 101,
 102, 103, 104, 105, 107, 108, 109, 110,
 111, 112, 113, 115, 116, 117, 118, 119,
 120, 121, 123, 124, 125, 126, 127, 128,
};

static const signed short greenAdjust1[] = {
  34,  34,  33,  33,  32,  32,  32,  31,
  31,  30,  30,  30,  29,  29,  28,  28,
  28,  27,  27,  27,  26,  26,  25,  25,
  25,  24,  24,  23,  23,  23,  22,  22,
  21,  21,  21,  20,  20,  19,  19,  19,
  18,  18,  17,  17,  17,  16,  16,  15,
  15,  15,  14,  14,  13,  13,  13,  12,
  12,  12,  11,  11,  10,  10,  10,   9,
   9,   8,   8,   8,   7,   7,   6,   6,
   6,   5,   5,   4,   4,   4,   3,   3,
   2,   2,   2,   1,   1,   0,   0,   0,
   0,   0,  -1,  -1,  -1,  -2,  -2,  -2,
  -3,  -3,  -4,  -4,  -4,  -5,  -5,  -6,
  -6,  -6,  -7,  -7,  -8,  -8,  -8,  -9,
  -9, -10, -10, -10, -11, -11, -12, -12,
 -12, -13, -13, -14, -14, -14, -15, -15,
 -16, -16, -16, -17, -17, -17, -18, -18,
 -19, -19, -19, -20, -20, -21, -21, -21,
 -22, -22, -23, -23, -23, -24, -24, -25,
 -25, -25, -26, -26, -27, -27, -27, -28,
 -28, -29, -29, -29, -30, -30, -30, -31,
 -31, -32, -32, -32, -33, -33, -34, -34,
 -34, -35, -35, -36, -36, -36, -37, -37,
 -38, -38, -38, -39, -39, -40, -40, -40,
 -41, -41, -42, -42, -42, -43, -43, -44,
 -44, -44, -45, -45, -45, -46, -46, -47,
 -47, -47, -48, -48, -49, -49, -49, -50,
 -50, -51, -51, -51, -52, -52, -53, -53,
 -53, -54, -54, -55, -55, -55, -56, -56,
 -57, -57, -57, -58, -58, -59, -59, -59,
 -60, -60, -60, -61, -61, -62, -62, -62,
 -63, -63, -64, -64, -64, -65, -65, -66,
};

static const signed short greenAdjust2[] = {
  74,  73,  73,  72,  71,  71,  70,  70,
  69,  69,  68,  67,  67,  66,  66,  65,
  65,  64,  63,  63,  62,  62,  61,  60,
  60,  59,  59,  58,  58,  57,  56,  56,
  55,  55,  54,  53,  53,  52,  52,  51,
  51,  50,  49,  49,  48,  48,  47,  47,
  46,  45,  45,  44,  44,  43,  42,  42,
  41,  41,  40,  40,  39,  38,  38,  37,
  37,  36,  35,  35,  34,  34,  33,  33,
  32,  31,  31,  30,  30,  29,  29,  28,
  27,  27,  26,  26,  25,  24,  24,  23,
  23,  22,  22,  21,  20,  20,  19,  19,
  18,  17,  17,  16,  16,  15,  15,  14,
  13,  13,  12,  12,  11,  11,  10,   9,
   9,   8,   8,   7,   6,   6,   5,   5,
   4,   4,   3,   2,   2,   1,   1,   0,
   0,   0,  -1,  -1,  -2,  -2,  -3,  -4,
  -4,  -5,  -5,  -6,  -6,  -7,  -8,  -8,
  -9,  -9, -10, -11, -11, -12, -12, -13,
 -13, -14, -15, -15, -16, -16, -17, -17,
 -18, -19, -19, -20, -20, -21, -22, -22,
 -23, -23, -24, -24, -25, -26, -26, -27,
 -27, -28, -29, -29, -30, -30, -31, -31,
 -32, -33, -33, -34, -34, -35, -35, -36,
 -37, -37, -38, -38, -39, -40, -40, -41,
 -41, -42, -42, -43, -44, -44, -45, -45,
 -46, -47, -47, -48, -48, -49, -49, -50,
 -51, -51, -52, -52, -53, -53, -54, -55,
 -55, -56, -56, -57, -58, -58, -59, -59,
 -60, -60, -61, -62, -62, -63, -63, -64,
 -65, -65, -66, -66, -67, -67, -68, -69,
 -69, -70, -70, -71, -71, -72, -73, -73,
};

static const signed short blueAdjust[] = {
-276,-274,-272,-270,-267,-265,-263,-261,
-259,-257,-255,-253,-251,-249,-247,-245,
-243,-241,-239,-237,-235,-233,-231,-229,
-227,-225,-223,-221,-219,-217,-215,-213,
-211,-209,-207,-204,-202,-200,-198,-196,
-194,-192,-190,-188,-186,-184,-182,-180,
-178,-176,-174,-172,-170,-168,-166,-164,
-162,-160,-158,-156,-154,-152,-150,-148,
-146,-144,-141,-139,-137,-135,-133,-131,
-129,-127,-125,-123,-121,-119,-117,-115,
-113,-111,-109,-107,-105,-103,-101, -99,
 -97, -95, -93, -91, -89, -87, -85, -83,
 -81, -78, -76, -74, -72, -70, -68, -66,
 -64, -62, -60, -58, -56, -54, -52, -50,
 -48, -46, -44, -42, -40, -38, -36, -34,
 -32, -30, -28, -26, -24, -22, -20, -18,
 -16, -13, -11,  -9,  -7,  -5,  -3,  -1,
   0,   2,   4,   6,   8,  10,  12,  14,
  16,  18,  20,  22,  24,  26,  28,  30,
  32,  34,  36,  38,  40,  42,  44,  46,
  49,  51,  53,  55,  57,  59,  61,  63,
  65,  67,  69,  71,  73,  75,  77,  79,
  81,  83,  85,  87,  89,  91,  93,  95,
  97,  99, 101, 103, 105, 107, 109, 112,
 114, 116, 118, 120, 122, 124, 126, 128,
 130, 132, 134, 136, 138, 140, 142, 144,
 146, 148, 150, 152, 154, 156, 158, 160,
 162, 164, 166, 168, 170, 172, 175, 177,
 179, 181, 183, 185, 187, 189, 191, 193,
 195, 197, 199, 201, 203, 205, 207, 209,
 211, 213, 215, 217, 219, 221, 223, 225,
 227, 229, 231, 233, 235, 238, 240, 242,
};

static const unsigned char jpeg_header[JPEG_HEADER_LEN] =
{
      // APP0
      0xff, 0xd8, 0xff, 0xe0, 0x0, 0x10, 0x4a, 0x46, // 0-7
      0x49, 0x46, 0x0, 0x1, 0x1, 0x1, 0x0, 0x48, // 8 - 15
      0x0, 0x48, 0x0, 0x0,

      // luma quant
      0xff, 0xdb, 0x0, 0x43, 0x0,
      0x3, 0x2, 0x2, 0x2, 0x2, 0x3, 0x3, 0x2,
      0x3, 0x3, 0x5, 0x4, 0x3, 0x3, 0x3, 0x5,
      0x4, 0x4, 0x5, 0x5, 0x8, 0xa, 0xb, 0x8,
      0x5, 0x7, 0x7, 0x9, 0xe, 0xc, 0xa, 0xb,
      0xa, 0xb, 0xb, 0xb, 0xa, 0xd, 0x10, 0xd,
      0xc, 0xf, 0x11, 0x12, 0x10, 0xf, 0x14, 0xf,
      0xb, 0xc, 0x13, 0x14, 0x13, 0x12, 0x15, 0x17,
      0x15, 0xe, 0x11, 0x17, 0x13, 0x13, 0x13, 0x13,


      // chrome quant
      0xff, 0xdb, 0x0, 0x43, 0x1, 0x3, 0x3, 0x3,
      0x5, 0x4, 0x5, 0x9, 0x5, 0x5, 0x9, 0x13,
      0xc, 0xb, 0xc, 0x13, 0x13, 0x13, 0x13, 0x13,
      0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
      0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
      0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
      0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
      0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
      0x13, 0x13, 0x13, 0x13, 0x13,

      // SOF0
      0xff, 0xc0, // 152 - 159
      0x0, 0x11, 0x8, 0x4, 0xb0, 0x6, 0x40, 0x3, // 160 -- 167
      0x1, 0x21, 0x0, 0x2, 0x11, 0x1, 0x3, 0x11,
      0x1,

      0xff, 0xc4, 0x0, 0x1f, 0x0, 0x0, 0x1,
      0x5, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x0,
      0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1,
      0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9,
      0xa, 0xb,

      0xff, 0xc4, 0x0, 0xb5, 0x10, 0x0,
      0x2, 0x1, 0x3, 0x3, 0x2, 0x4, 0x3, 0x5,
      0x5, 0x4, 0x4, 0x0, 0x0, 0x1, 0x7d, 0x1,
      0x2, 0x3, 0x0, 0x4, 0x11, 0x5, 0x12, 0x21,
      0x31, 0x41, 0x6, 0x13, 0x51, 0x61, 0x7, 0x22,
      0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x8, 0x23,
      0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0, 0x24,
      0x33, 0x62, 0x72, 0x82, 0x9, 0xa, 0x16, 0x17,
      0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28, 0x29,
      0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a,
      0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,
      0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a,
      0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a,
      0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,
      0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a,
      0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99,
      0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8,
      0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
      0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6,
      0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5,
      0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2, 0xe3,
      0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf1,
      0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9,
      0xfa,

      0xff, 0xc4, 0x0, 0x1f, 0x1, 0x0, 0x3,
      0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
      0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1,
      0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9,
      0xa, 0xb,

      0xff, 0xc4, 0x0, 0xb5, 0x11, 0x0,
      0x2, 0x1, 0x2, 0x4, 0x4, 0x3, 0x4, 0x7,
      0x5, 0x4, 0x4, 0x0, 0x1, 0x2, 0x77, 0x0,
      0x1, 0x2, 0x3, 0x11, 0x4, 0x5, 0x21, 0x31,
      0x6, 0x12, 0x41, 0x51, 0x7, 0x61, 0x71, 0x13,
      0x22, 0x32, 0x81, 0x8, 0x14, 0x42, 0x91, 0xa1,
      0xb1, 0xc1, 0x9, 0x23, 0x33, 0x52, 0xf0, 0x15,
      0x62, 0x72, 0xd1, 0xa, 0x16, 0x24, 0x34, 0xe1,
      0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26, 0x27,
      0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38, 0x39,
      0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
      0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
      0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
      0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
      0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88,
      0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
      0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6,
      0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5,
      0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4,
      0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3,
      0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe2,
      0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
      0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9,
      0xfa,

      0xff, 0xda, 0x0, 0xc, 0x3, 0x1, 0x0,
      0x2, 0x11, 0x3, 0x11, 0x0, 0x3f, 0x0,
  };

static const unsigned char jpeg_tail[JPEG_TAIL_LEN] = { 0xFF, 0xD9 };


#define CLAMP(x) x < 0 ? 0 : x & 0xff

inline void yuv2rgb(int y, int u, int v, quint16 *rgb)
{
    register  int r, g, b;

    r = y + redAdjust[v];
    g = y + greenAdjust1[u] + greenAdjust2[v];
    b = y + blueAdjust[u];

    r = CLAMP(r);
    g = CLAMP(g);
    b = CLAMP(b);

    *rgb = (quint16)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
}


void EzxCamera::getCameraImage( QImage& img, bool copy )
{
    Q_UNUSED(copy);

    int ret;

    if ( fd == -1 ) {
        if ( img.isNull() ) {
            img = QImage(height, width, QImage::Format_RGB16);
        }
        return;
    }

    fd_set rfds;
    FD_ZERO (&rfds);
    FD_SET (fd, &rfds);

    select (fd+1, &rfds, NULL, NULL, NULL);

    static int _lastFrame = 1;

    struct V4l_IMAGE_FRAME frame;

    ret = ioctl(fd, WCAM_VIDIOCGRABFRAME, &frame);
    if (ret)
      qWarning("cant grab frame");

    if (width < 320) {
      int currentFrame = (_lastFrame - 1) % mbuf.frames;
      unsigned char *buf_orig = frames + mbuf.offsets[currentFrame];
      for(int yy_base = 0; yy_base < width; yy_base += 64)
      {
          for(int x = 0; x < height && x < width; ++x)
          {
              for (int y = yy_base; y < width && y < yy_base + 64; y++)
              {
                  unsigned short *dest = (unsigned short*)m_imageBuf + x + y * height;
                  unsigned char *buf = buf_orig + ((height - 1) - x) * 2 * width + (y & 0xFFFFFFFE) * 2;
                  int u = buf[0];
                  int v = buf[2];
                  yuv2rgb(buf[1 + (y & 0x1) * 2], u, v, dest);
              }
          }
      }
    } else {

      unsigned char *dst = (unsigned char*)m_imageBuf;
      memcpy(dst,jpeg_header,JPEG_HEADER_LEN);

      dst[163] = (unsigned char)(height >> 8);
      dst[164] = (unsigned char)(height & 0xFF);

      dst[165] = (unsigned char)(width >> 8);
      dst[166] = (unsigned char)(width & 0xFF);

      dst += JPEG_HEADER_LEN;
      memcpy(dst,frames,frame.planeBytes[0]);
      dst += frame.planeBytes[0];
      memcpy(dst,jpeg_tail,JPEG_TAIL_LEN);


      qDebug() << "size: " << JPEG_HEADER_LEN << frame.planeBytes[0] << JPEG_TAIL_LEN;

      int dump = open("/tmp/dump.jpeg",O_RDWR | O_CREAT);
      write(dump,m_imageBuf,JPEG_HEADER_LEN + frame.planeBytes[0] + JPEG_TAIL_LEN);
      close(dump);

    }

    int cap = 1;

    ret =ioctl(fd, WCAM_VIDIOCNEXTFRAME, &cap);
    if (ret)
      qWarning("cant switch to next frame");

    if(cap != _lastFrame)
        _lastFrame = cap;

    if (width > 320) {
      QPixmap pix;
      qDebug() << "pixmap" <<  pix.loadFromData(
          (uchar*)m_imageBuf,
          JPEG_HEADER_LEN + JPEG_TAIL_LEN + frame.planeBytes[0],
          "JPEG"
      );
      img = pix.toImage();
    } else {
      img = QImage((uchar*) m_imageBuf, height, width, QImage::Format_RGB16);
    }

}

QList<QSize> EzxCamera::photoSizes() const
{
    QList<QSize> list;

    list << QSize(176, 176) << QSize(160, 120) <<
            QSize(176, 176) <<
            QSize(320, 240) << QSize(352, 288) <<
            QSize(640, 480) << QSize(1280, 1024);

    return list;
}

QList<QSize> EzxCamera::videoSizes() const
{
    QList<QSize> list;
    list << QSize(176, 176) << QSize(176, 176) << QSize(320, 240) ;
    return list;
}

QSize EzxCamera::recommendedPhotoSize() const
{
    return QSize(480, 640);
}

QSize EzxCamera::recommendedVideoSize() const
{
    return QSize(480, 640);
}

QSize EzxCamera::recommendedPreviewSize() const
{
    return QSize(176, 176);
}

void EzxCamera::setCaptureSize( QSize size )
{
    qDebug() << "set size" <<  size.width() << size.height();

    stop();
    setSize( size );

    if (size.width() > 320)
      photo();
    else
      start();
}

} // ns camera

