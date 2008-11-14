/****************************************************************************
**
** (c) 2008. Ilya Petrov <ilya.muromec@gmail.com> 
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qtopianamespace.h>
#include <custom.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <unistd.h>


#define FRAMEBUFFER_DEVICE "/dev/fb0"
#define KEYLIGHT_DEVICE "/dev/keylight"

#define PWMLED_0_ON             0xf0
#define PWMLED_0_OFF            0xf2
#define FRAMEBUFFER_DIM 11
#define FRAMEBUFFER_MAX 100

bool fb_disp_on;

QTOPIABASE_EXPORT int qpe_sysBrightnessSteps()
{
    return FRAMEBUFFER_MAX;
}

QTOPIABASE_EXPORT void qpe_setBrightness(int b)
{
  int fbh, kbh, kbvalue;
  int ret;

  fbh = open(FRAMEBUFFER_DEVICE, O_RDWR); // lcd
  kbh = open(KEYLIGHT_DEVICE,    O_RDWR); // keyboard


  if (b) {

    kbvalue = PWMLED_0_ON;

    if (b > FRAMEBUFFER_MAX) { // normalize
      b = FRAMEBUFFER_MAX ;
    } else if (b == 1) { // dim
      b = FRAMEBUFFER_DIM;
      kbvalue = PWMLED_0_OFF;
    } 

    // if no power - set it
    if (! fb_disp_on ) { 
      ret = ioctl(fbh, FBIOBLANK, VESA_NO_BLANKING);
      ret = ioctl(fbh, FBIOSETBKLIGHT, BKLIGHT_ON);

      fb_disp_on = true;
    }
  } else {
    kbvalue = PWMLED_0_OFF;

    // power down lcd to save power
    ret = ioctl(fbh, FBIOSETBKLIGHT, BKLIGHT_OFF);
    ret = ioctl(fbh, FBIOBLANK, VESA_POWERDOWN);

    fb_disp_on = false;
  }

  ret = ioctl(fbh, FBIOSETBRIGHTNESS, b);
  ret = ioctl(kbh, kbvalue, 1);

  close(fbh);
  close(kbh);

}

