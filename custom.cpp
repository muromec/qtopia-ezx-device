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


#define FRAMEBUFFER_DEVICE "/dev/fb0"

QTOPIABASE_EXPORT int qpe_sysBrightnessSteps()
{
    // FIXME
    return 20;
}

QTOPIABASE_EXPORT void qpe_setBrightness(int b)
{
  // TODO: what is 2463 and how to hadle it?
  // TODO: handle 0 as backlight 
  int fbh;
  int ret;
  fbh = open(FRAMEBUFFER_DEVICE, O_RDWR);
  ret = ioctl(fbh, FBIOSETBRIGHTNESS, b);

}

