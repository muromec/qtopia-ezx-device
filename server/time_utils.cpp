#include "config.h"

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>


#include "time_utils.h"

void Time::save()
{
  struct timeval tv = { 0, 0 };
  struct timezone tz;

  gettimeofday(&tv,&tz);

  // write time to pcap
  int pcap_rtc = open("/dev/pcap_rtc", O_RDWR);
  ioctl(pcap_rtc, POWER_IC_IOCTL_SET_TIME, &tv);
  close(pcap_rtc);
}

void Time::load()
{
  struct timeval tv = { 0, 0 };
  struct timezone tz;

  // read timezone
  gettimeofday(&tv,&tz);

  // read time from pcap
  int pcap_rtc = open("/dev/pcap_rtc", O_RDWR);
  int ret = ioctl(pcap_rtc, POWER_IC_IOCTL_GET_TIME, &tv);
  close(pcap_rtc);

  if (!ret)
    settimeofday(&tv,&tz); // set system time
  else
    err("Agggrrhhh... POWER_IC_IOCTL_GET_TIME failed");
}

int Time::currentStamp()
{
  struct timeval tv = { 0, 0 };
  struct timezone tz;

  // read timezone
  gettimeofday(&tv,&tz);
  return tv.tv_sec;
};
