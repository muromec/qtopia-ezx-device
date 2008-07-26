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

bool fb_disp_on;

QTOPIABASE_EXPORT int qpe_sysBrightnessSteps()
{
    // FIXME
    return 100;
}

QTOPIABASE_EXPORT void qpe_setBrightness(int b)
{
  // TODO: notify apmd
  int fbh;
  int ret;
  fbh = open(FRAMEBUFFER_DEVICE, O_RDWR);

  if (b > 100)
    b = 100;

  if (b > 1) {

    if (! fb_disp_on ) {
      ret = ioctl(fbh, FBIOBLANK, VESA_NO_BLANKING);
      printf("FBIOBLANK %d %d\n", ret, VESA_NO_BLANKING);
      ret = ioctl(fbh, FBIOSETBKLIGHT, BKLIGHT_ON);
      printf("FBIOSETBKLIGHT %d\n",ret);

      fb_disp_on = true;
    }
    ret = ioctl(fbh, FBIOSETBRIGHTNESS, b);
  } else {
    // power down lcd to save power
    ret = ioctl(fbh, FBIOSETBRIGHTNESS, 0); 
    printf("FBIOSETBRIGHTNESS-0 %d\n",ret);

    ret = ioctl(fbh, FBIOSETBKLIGHT, BKLIGHT_OFF);
    printf("FBIOSETBKLIGHT ret: %d, %d\n", ret, BKLIGHT_OFF);

    ret = ioctl(fbh, FBIOBLANK, VESA_POWERDOWN);
    printf("FBIOBLANK ret: %d, %d\n", ret, VESA_POWERDOWN);
    fb_disp_on = false;
  }




  close(fbh);

}

