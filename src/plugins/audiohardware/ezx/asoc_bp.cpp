#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include "asoc_bp.h"
#include "qaudiooutput.h"
	      
static snd_pcm_t *playback_handle = NULL;
static short ap_opened = 0;

int open_bp () 
{
  int err;
  snd_pcm_hw_params_t *hw_params;

  if (__output and __output->isOpen()) {
    printf("ap opened! close ap and open bp\n");
    ap_opened = (short)__output->openMode();
    printf("open mode was %d\n", ap_opened);
    __output->close();
  }

  if ((err = snd_pcm_open (&playback_handle, ASOC_BP_DEV, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
    fprintf (stderr, "cannot open audio device %s (%s)\n", 
        ASOC_BP_DEV,
        snd_strerror (err));

    return 1;
  }
  printf("snd_pcm_open\n");

  if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
    fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
        snd_strerror (err));

    return 1;
  }
  printf("snd_pcm_hw_params_malloc\n");

  if ((err = snd_pcm_hw_params_any (playback_handle, hw_params)) < 0) {
    fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
        snd_strerror (err));

    return 1;
  }
  printf("snd_pcm_hw_params_any\n");

  if ((err = snd_pcm_hw_params_set_access (playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    fprintf (stderr, "cannot set access type (%s)\n",
        snd_strerror (err));
    return 1;
  }
  printf("snd_pcm_hw_params_set_access\n");

  if ((err = snd_pcm_hw_params_set_format (playback_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
    fprintf (stderr, "cannot set sample format (%s)\n",
        snd_strerror (err));
    return 1;
  }
  printf("snd_pcm_hw_params_set_format\n");
  unsigned int rate = 8000;

  if ((err = snd_pcm_hw_params_set_rate_near (playback_handle, hw_params, &rate, 0)) < 0) {
    printf("fuck\n");
    fprintf (stderr, "cannot set sample rate (%s)\n",
        snd_strerror (err));
    return 1;
  }
  printf("snd_pcm_hw_params_set_rate_near\n");

  if ((err = snd_pcm_hw_params_set_channels (playback_handle, hw_params, 1)) < 0) {
    fprintf (stderr, "cannot set channel count (%s)\n",
        snd_strerror (err));

    return 1;
  }
  printf("snd_pcm_hw_params_set_channels\n");

  if ((err = snd_pcm_hw_params (playback_handle, hw_params)) < 0) {
    fprintf (stderr, "cannot set parameters (%s)\n",
        snd_strerror (err));

    return 1;
  }
  printf("snd_pcm_hw_params\n");


  return 0;
}

int close_bp() {

  if (playback_handle)
    snd_pcm_close (playback_handle);

  playback_handle = NULL;

  printf("open mode was %d, out %x\n", ap_opened, __output);
  if (__output and ap_opened) {
    printf("close bp, ap was opened\n");
    __output->open( (QIODevice::OpenMode)ap_opened );
    ap_opened = 0;
  }

  return 0;
}
