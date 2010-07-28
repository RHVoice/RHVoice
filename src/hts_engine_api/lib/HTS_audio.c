/* ----------------------------------------------------------------- */
/*           The HMM-Based Speech Synthesis System (HTS)             */
/*           hts_engine API developed by HTS Working Group           */
/*           http://hts-engine.sourceforge.net/                      */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2001-2010  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                                                                   */
/*                2001-2008  Tokyo Institute of Technology           */
/*                           Interdisciplinary Graduate School of    */
/*                           Science and Engineering                 */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the HTS working group nor the names of its  */
/*   contributors may be used to endorse or promote products derived */
/*   from this software without specific prior written permission.   */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

/* hts_engine libralies */
#include "HTS_hidden.h"

/* for windows & windows mobile */
#if defined(AUDIO_PLAY_WIN32) || defined(AUDIO_PLAY_WINCE)

#define AUDIO_WAIT_BUFF_MS 10   /* wait time (ms) */
#define AUDIO_CHANNEL 1         /* monoral */

/* HTS_Audio_callback_function: callback function from audio device */
static void CALLBACK HTS_Audio_callback_function(HWAVEOUT hwaveout, UINT msg,
                                                 DWORD user_data, DWORD param1,
                                                 DWORD param2)
{
   WAVEHDR *wavehdr = (WAVEHDR *) param1;
   HTS_Audio *as = (HTS_Audio *) user_data;

   if (msg == MM_WOM_DONE && wavehdr && (wavehdr->dwFlags & WHDR_DONE)) {
      if (as->now_buff_1 == TRUE && wavehdr == &(as->buff_1)) {
         as->now_buff_1 = FALSE;
      } else if (as->now_buff_2 == TRUE && wavehdr == &(as->buff_2)) {
         as->now_buff_2 = FALSE;
      }
   }
}

/* HTS_Audio_write_buffer: send buffer to audio device */
static void HTS_Audio_write_buffer(HTS_Audio * as)
{
   MMRESULT error;

   if (as->which_buff == 1) {
      while (as->now_buff_1 == TRUE)
         Sleep(AUDIO_WAIT_BUFF_MS);
      as->now_buff_1 = TRUE;
      as->which_buff = 2;
      memcpy(as->buff_1.lpData, as->buff, as->buff_size * sizeof(short));
      as->buff_1.dwBufferLength = as->buff_size * sizeof(short);
      error = waveOutWrite(as->hwaveout, &(as->buff_1), sizeof(WAVEHDR));
   } else {
      while (as->now_buff_2 == TRUE)
         Sleep(AUDIO_WAIT_BUFF_MS);
      as->now_buff_2 = TRUE;
      as->which_buff = 1;
      memcpy(as->buff_2.lpData, as->buff, as->buff_size * sizeof(short));
      as->buff_2.dwBufferLength = as->buff_size * sizeof(short);
      error = waveOutWrite(as->hwaveout, &(as->buff_2), sizeof(WAVEHDR));
   }

   if (error != MMSYSERR_NOERROR)
      HTS_error(0,
                "hts_engine: Cannot send datablocks to your output audio device to play waveform.\n");
}

/* HTS_Audio_open: open audio device */
void HTS_Audio_open(HTS_Audio * as, int sampling_rate, int max_buff_size)
{
   MMRESULT error;

   /* queue */
   as->which_buff = 1;
   as->now_buff_1 = FALSE;
   as->now_buff_2 = FALSE;
   as->max_buff_size = max_buff_size;
   as->buff = (short *) HTS_calloc(max_buff_size, sizeof(short));

   /* format */
   as->waveformatex.wFormatTag = WAVE_FORMAT_PCM;
   as->waveformatex.nChannels = AUDIO_CHANNEL;
   as->waveformatex.nSamplesPerSec = sampling_rate;
   as->waveformatex.wBitsPerSample = sizeof(short) * 8;
   as->waveformatex.nBlockAlign = AUDIO_CHANNEL
       * as->waveformatex.wBitsPerSample / 8;
   as->waveformatex.nAvgBytesPerSec = sampling_rate
       * as->waveformatex.nBlockAlign;
   /* open */
   error =
       waveOutOpen(&as->hwaveout, WAVE_MAPPER, &as->waveformatex,
                   (DWORD) HTS_Audio_callback_function, (DWORD) as,
                   CALLBACK_FUNCTION);
   if (error != MMSYSERR_NOERROR)
      HTS_error(0,
                "hts_engine: Failed to open your output audio device to play waveform.\n");

   /* prepare */
   as->buff_1.lpData = (LPSTR) HTS_calloc(max_buff_size, sizeof(short));
   as->buff_1.dwBufferLength = max_buff_size * sizeof(short);
   as->buff_1.dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;
   as->buff_1.dwLoops = 1;
   as->buff_1.lpNext = 0;
   as->buff_1.reserved = 0;
   error = waveOutPrepareHeader(as->hwaveout, &(as->buff_1), sizeof(WAVEHDR));
   if (error != MMSYSERR_NOERROR)
      HTS_error(0,
                "hts_engine: Cannot initialize audio datablocks to play waveform.\n");
   as->buff_2.lpData = (LPSTR) HTS_calloc(max_buff_size, sizeof(short));
   as->buff_2.dwBufferLength = max_buff_size * sizeof(short);
   as->buff_2.dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;
   as->buff_2.dwLoops = 1;
   as->buff_2.lpNext = 0;
   as->buff_2.reserved = 0;
   error = waveOutPrepareHeader(as->hwaveout, &(as->buff_2), sizeof(WAVEHDR));
   if (error != MMSYSERR_NOERROR)
      HTS_error(0,
                "hts_engine: Cannot initialize audio datablocks to play waveform.\n");
}

/* HTS_Audio_write: send data to audio device */
void HTS_Audio_write(HTS_Audio * as, short data)
{
   as->buff[as->buff_size] = data;
   as->buff_size++;
   if (as->buff_size == as->max_buff_size) {
      HTS_Audio_write_buffer(as);
      as->buff_size = 0;
   }
}

/* HTS_Audio_close: close audio device */
void HTS_Audio_close(HTS_Audio * as)
{
   MMRESULT error;

   if (as->buff_size != 0)
      HTS_Audio_write_buffer(as);
   while (as->now_buff_1 == TRUE)
      Sleep(AUDIO_WAIT_BUFF_MS);
   while (as->now_buff_2 == TRUE)
      Sleep(AUDIO_WAIT_BUFF_MS);
   /* stop audio */
   error = waveOutReset(as->hwaveout);
   if (error != MMSYSERR_NOERROR)
      HTS_error(0,
                "hts_engine: Cannot stop and reset your output audio device.\n");
   /* unprepare */
   error = waveOutUnprepareHeader(as->hwaveout, &(as->buff_1), sizeof(WAVEHDR));
   if (error != MMSYSERR_NOERROR)
      HTS_error(0,
                "hts_engine: Cannot cleanup the audio datablocks to play waveform.\n");
   error = waveOutUnprepareHeader(as->hwaveout, &(as->buff_2), sizeof(WAVEHDR));
   if (error != MMSYSERR_NOERROR)
      HTS_error(0,
                "hts_engine: Cannot cleanup the audio datablocks to play waveform.\n");
   /* close */
   error = waveOutClose(as->hwaveout);
   if (error != MMSYSERR_NOERROR)
      HTS_error(0, "hts_engine: Failed to close your output audio device.\n");
   HTS_free(as->buff_1.lpData);
   HTS_free(as->buff_2.lpData);
   HTS_free(as->buff);
}
#endif                          /* AUDIO_PLAY_WIN32 || AUDIO_PLAY_WINCE */

/* for ALSA on Linux */
#ifdef AUDIO_PLAY_ALSA
/* HTS_Audio_open: open audio device (dummy) */
void HTS_Audio_open(HTS_Audio * as, int sampling_rate, int max_buff_size)
{
}

/* HTS_Audio_write: send data to audio device (dummy) */
void HTS_Audio_write(HTS_Audio * as, short data)
{
}

/* HTS_Audio_close: close audio device */
void HTS_Audio_close(HTS_Audio * as)
{
}
#endif                          /* AUDIO_PLAY_ALSA */

/* for others */
#ifdef AUDIO_PLAY_NONE
/* HTS_Audio_open: open audio device (dummy) */
void HTS_Audio_open(HTS_Audio * as, int sampling_rate, int max_buff_size)
{
}

/* HTS_Audio_write: send data to audio device (dummy) */
void HTS_Audio_write(HTS_Audio * as, short data)
{
}

/* HTS_Audio_close: close audio device (dummy) */
void HTS_Audio_close(HTS_Audio * as)
{
}
#endif                          /* AUDIO_PLAY_NONE */
