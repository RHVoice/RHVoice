/* ----------------------------------------------------------------- */
/*           The HMM-Based Speech Synthesis Engine "hts_engine API"  */
/*           developed by HTS Working Group                          */
/*           http://hts-engine.sourceforge.net/                      */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2001-2011  Nagoya Institute of Technology          */
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

#ifndef HTS_AUDIO_C
#define HTS_AUDIO_C

#ifdef __cplusplus
#define HTS_AUDIO_C_START extern "C" {
#define HTS_AUDIO_C_END   }
#else
#define HTS_AUDIO_C_START
#define HTS_AUDIO_C_END
#endif                          /* __CPLUSPLUS */

HTS_AUDIO_C_START;

/* hts_engine libralies */
#include "HTS_hidden.h"

/* for windows & windows mobile */
#if defined(AUDIO_PLAY_WIN32) || defined(AUDIO_PLAY_WINCE)

#define AUDIO_WAIT_BUFF_MS 10   /* wait time (ms) */
#define AUDIO_CHANNEL 1         /* monoral */

/* HTS_Audio_callback_function: callback function from audio device */
static void CALLBACK HTS_Audio_callback_function(HWAVEOUT hwaveout, UINT msg, DWORD user_data, DWORD param1, DWORD param2)
{
   WAVEHDR *wavehdr = (WAVEHDR *) param1;
   HTS_Audio *audio = (HTS_Audio *) user_data;

   if (msg == MM_WOM_DONE && wavehdr && (wavehdr->dwFlags & WHDR_DONE)) {
      if (audio->now_buff_1 == TRUE && wavehdr == &(audio->buff_1)) {
         audio->now_buff_1 = FALSE;
      } else if (audio->now_buff_2 == TRUE && wavehdr == &(audio->buff_2)) {
         audio->now_buff_2 = FALSE;
      }
   }
}

/* HTS_Audio_write_buffer: send buffer to audio device */
static void HTS_Audio_write_buffer(HTS_Audio * audio)
{
   MMRESULT error;

   if (audio->which_buff == 1) {
      while (audio->now_buff_1 == TRUE)
         Sleep(AUDIO_WAIT_BUFF_MS);
      audio->now_buff_1 = TRUE;
      audio->which_buff = 2;
      memcpy(audio->buff_1.lpData, audio->buff, audio->buff_size * sizeof(short));
      audio->buff_1.dwBufferLength = audio->buff_size * sizeof(short);
      error = waveOutWrite(audio->hwaveout, &(audio->buff_1), sizeof(WAVEHDR));
   } else {
      while (audio->now_buff_2 == TRUE)
         Sleep(AUDIO_WAIT_BUFF_MS);
      audio->now_buff_2 = TRUE;
      audio->which_buff = 1;
      memcpy(audio->buff_2.lpData, audio->buff, audio->buff_size * sizeof(short));
      audio->buff_2.dwBufferLength = audio->buff_size * sizeof(short);
      error = waveOutWrite(audio->hwaveout, &(audio->buff_2), sizeof(WAVEHDR));
   }

   if (error != MMSYSERR_NOERROR)
      HTS_error(0, "hts_engine: Cannot send datablocks to your output audio device to play waveform.\n");
}

/* HTS_Audio_close: close audio device */
static void HTS_Audio_close(HTS_Audio * audio)
{
   MMRESULT error;

   if (audio->max_buff_size <= 0)
      return;

   /* stop audio */
   error = waveOutReset(audio->hwaveout);
   if (error != MMSYSERR_NOERROR)
      HTS_error(0, "hts_engine: Cannot stop and reset your output audio device.\n");
   /* unprepare */
   error = waveOutUnprepareHeader(audio->hwaveout, &(audio->buff_1), sizeof(WAVEHDR));
   if (error != MMSYSERR_NOERROR)
      HTS_error(0, "hts_engine: Cannot cleanup the audio datablocks to play waveform.\n");
   error = waveOutUnprepareHeader(audio->hwaveout, &(audio->buff_2), sizeof(WAVEHDR));
   if (error != MMSYSERR_NOERROR)
      HTS_error(0, "hts_engine: Cannot cleanup the audio datablocks to play waveform.\n");
   /* close */
   error = waveOutClose(audio->hwaveout);
   if (error != MMSYSERR_NOERROR)
      HTS_error(0, "hts_engine: Failed to close your output audio device.\n");
   HTS_free(audio->buff_1.lpData);
   HTS_free(audio->buff_2.lpData);
   HTS_free(audio->buff);
}

/* HTS_Audio_open: open audio device */
static void HTS_Audio_open(HTS_Audio * audio, int sampling_rate, int max_buff_size)
{
   MMRESULT error;

   if (audio->sampling_rate == sampling_rate && audio->max_buff_size == max_buff_size)
      return;

   HTS_Audio_close(audio);

   audio->sampling_rate = sampling_rate;
   audio->max_buff_size = max_buff_size;

   if (audio->max_buff_size <= 0)
      return;

   /* queue */
   audio->which_buff = 1;
   audio->now_buff_1 = FALSE;
   audio->now_buff_2 = FALSE;
   audio->buff = (short *) HTS_calloc(max_buff_size, sizeof(short));

   /* format */
   audio->waveformatex.wFormatTag = WAVE_FORMAT_PCM;
   audio->waveformatex.nChannels = AUDIO_CHANNEL;
   audio->waveformatex.nSamplesPerSec = sampling_rate;
   audio->waveformatex.wBitsPerSample = sizeof(short) * 8;
   audio->waveformatex.nBlockAlign = AUDIO_CHANNEL * audio->waveformatex.wBitsPerSample / 8;
   audio->waveformatex.nAvgBytesPerSec = sampling_rate * audio->waveformatex.nBlockAlign;
   /* open */
   error = waveOutOpen(&audio->hwaveout, WAVE_MAPPER, &audio->waveformatex, (DWORD) HTS_Audio_callback_function, (DWORD) audio, CALLBACK_FUNCTION);
   if (error != MMSYSERR_NOERROR)
      HTS_error(0, "hts_engine: Failed to open your output audio device to play waveform.\n");

   /* prepare */
   audio->buff_1.lpData = (LPSTR) HTS_calloc(max_buff_size, sizeof(short));
   audio->buff_1.dwBufferLength = max_buff_size * sizeof(short);
   audio->buff_1.dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;
   audio->buff_1.dwLoops = 1;
   audio->buff_1.lpNext = 0;
   audio->buff_1.reserved = 0;
   error = waveOutPrepareHeader(audio->hwaveout, &(audio->buff_1), sizeof(WAVEHDR));
   if (error != MMSYSERR_NOERROR)
      HTS_error(0, "hts_engine: Cannot initialize audio datablocks to play waveform.\n");
   audio->buff_2.lpData = (LPSTR) HTS_calloc(max_buff_size, sizeof(short));
   audio->buff_2.dwBufferLength = max_buff_size * sizeof(short);
   audio->buff_2.dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;
   audio->buff_2.dwLoops = 1;
   audio->buff_2.lpNext = 0;
   audio->buff_2.reserved = 0;
   error = waveOutPrepareHeader(audio->hwaveout, &(audio->buff_2), sizeof(WAVEHDR));
   if (error != MMSYSERR_NOERROR)
      HTS_error(0, "hts_engine: Cannot initialize audio datablocks to play waveform.\n");
}

/* HTS_Audio_initialize: initialize audio */
void HTS_Audio_initialize(HTS_Audio * audio, int sampling_rate, int max_buff_size)
{
   audio->sampling_rate = 0;
   audio->max_buff_size = 0;

   audio->hwaveout = 0;
   audio->buff = NULL;
   audio->buff_size = 0;
   audio->which_buff = 0;
   audio->now_buff_1 = FALSE;
   audio->now_buff_2 = FALSE;

   HTS_Audio_open(audio, sampling_rate, max_buff_size);
}

/* HTS_Audio_set_parameter: set parameters for audio */
void HTS_Audio_set_parameter(HTS_Audio * audio, int sampling_rate, int max_buff_size)
{
   HTS_Audio_open(audio, sampling_rate, max_buff_size);
}

/* HTS_Audio_write: send data to audio */
void HTS_Audio_write(HTS_Audio * audio, short data)
{
   audio->buff[audio->buff_size++] = data;
   if (audio->buff_size == audio->max_buff_size) {
      HTS_Audio_write_buffer(audio);
      audio->buff_size = 0;
   }
}

/* HTS_Audio_flush: flush remain data */
void HTS_Audio_flush(HTS_Audio * audio)
{
   if (audio->buff_size > 0) {
      HTS_Audio_write_buffer(audio);
      audio->buff_size = 0;
   }
   while (audio->now_buff_1 == TRUE)
      Sleep(AUDIO_WAIT_BUFF_MS);
   while (audio->now_buff_2 == TRUE)
      Sleep(AUDIO_WAIT_BUFF_MS);
}

/* HTS_Audio_clear: free audio */
void HTS_Audio_clear(HTS_Audio * audio)
{
   HTS_Audio_flush(audio);
   HTS_Audio_close(audio);
}

#endif                          /* AUDIO_PLAY_WIN32 || AUDIO_PLAY_WINCE */

#ifdef AUDIO_PLAY_PORTAUDIO

/* HTS_Audio_close: close audio device */
static void HTS_Audio_close(HTS_Audio * audio)
{
   if (audio->max_buff_size <= 0)
      return;

   if (audio->buff_size > 0) {
      audio->err = Pa_WriteStream(audio->stream, audio->buff, audio->buff_size);
      if (audio->err != paNoError && audio->err != paOutputUnderflowed)
         HTS_error(0, "hts_engine: Cannot send datablocks to your output audio device to play waveform.\n");
      audio->buff_size = 0;
   }

   HTS_free(audio->buff);

   audio->err = Pa_StopStream(audio->stream);
   if (audio->err != paNoError)
      HTS_error(0, "hts_engine: Cannot stop your output audio device.\n");

   audio->err = Pa_CloseStream(audio->stream);
   if (audio->err != paNoError)
      HTS_error(0, "hts_engine: Failed to close your output audio device.\n");

   Pa_Terminate();
}

/* HTS_Audio_open: open audio device */
static void HTS_Audio_open(HTS_Audio * audio, int sampling_rate, int max_buff_size)
{
   if (audio->sampling_rate == sampling_rate && audio->max_buff_size == max_buff_size)
      return;

   HTS_Audio_close(audio);

   audio->sampling_rate = sampling_rate;
   audio->max_buff_size = max_buff_size;

   if (audio->max_buff_size <= 0)
      return;

   audio->err = Pa_Initialize();
   if (audio->err != paNoError)
      HTS_error(0, "hts_engine: Failed to initialize your output audio device to play waveform.\n");

   audio->parameters.device = Pa_GetDefaultOutputDevice();
   audio->parameters.channelCount = 1;
   audio->parameters.sampleFormat = paInt16;
   audio->parameters.suggestedLatency = Pa_GetDeviceInfo(audio->parameters.device)->defaultLowOutputLatency;
   audio->parameters.hostApiSpecificStreamInfo = NULL;

   audio->err = Pa_OpenStream(&audio->stream, NULL, &audio->parameters, sampling_rate, max_buff_size, paClipOff, NULL, NULL);
   if (audio->err != paNoError)
      HTS_error(0, "hts_engine: Failed to open your output audio device to play waveform.\n");

   audio->err = Pa_StartStream(audio->stream);
   if (audio->err != paNoError)
      HTS_error(0, "hts_engine: Failed to start your output audio device to play waveform.\n");

   audio->buff = (short *) HTS_calloc(max_buff_size, sizeof(short));
   audio->buff_size = 0;
}

/* HTS_Audio_initialize: initialize audio */
void HTS_Audio_initialize(HTS_Audio * audio, int sampling_rate, int max_buff_size)
{
   audio->sampling_rate = 0;
   audio->max_buff_size = 0;

   audio->stream = NULL;
   audio->err = 0;
   audio->buff = NULL;
   audio->buff_size = 0;

   HTS_Audio_open(audio, sampling_rate, max_buff_size);
}

/* HTS_Audio_set_parameter: set parameters for audio */
void HTS_Audio_set_parameter(HTS_Audio * audio, int sampling_rate, int max_buff_size)
{
   HTS_Audio_open(audio, sampling_rate, max_buff_size);
}

/* HTS_Audio_write: send data to audio device */
void HTS_Audio_write(HTS_Audio * audio, short data)
{
   audio->buff[audio->buff_size++] = data;
   if (audio->buff_size >= audio->max_buff_size) {
      audio->err = Pa_WriteStream(audio->stream, audio->buff, audio->max_buff_size);
      if (audio->err != paNoError && audio->err != paOutputUnderflowed)
         HTS_error(0, "hts_engine: Cannot send datablocks to your output audio device to play waveform.\n");
      audio->buff_size = 0;
   }
}

/* HTS_Audio_flush: flush remain data */
void HTS_Audio_flush(HTS_Audio * audio)
{
   if (audio->buff_size > 0) {
      audio->err = Pa_WriteStream(audio->stream, audio->buff, audio->buff_size);
      if (audio->err != paNoError && audio->err != paOutputUnderflowed)
         HTS_error(0, "hts_engine: Cannot send datablocks to your output audio device to play waveform.\n");
      audio->buff_size = 0;
   }
}

/* HTS_Audio_clear: free audio */
void HTS_Audio_clear(HTS_Audio * audio)
{
   HTS_Audio_flush(audio);
   HTS_Audio_close(audio);
}

#endif                          /* AUDIO_PLAY_PORTAUDIO */

/* for others */
#ifdef AUDIO_PLAY_NONE
/* HTS_Audio_initialize: initialize audio */
void HTS_Audio_initialize(HTS_Audio * audio, int sampling_rate, int max_buff_size)
{
}

/* HTS_Audio_set_parameter: set parameters for audio */
void HTS_Audio_set_parameter(HTS_Audio * audio, int sampling_rate, int max_buff_size)
{
}

/* HTS_Audio_write: send data to audio */
void HTS_Audio_write(HTS_Audio * audio, short data)
{
}

/* HTS_Audio_flush: flush remain data */
void HTS_Audio_flush(HTS_Audio * audio)
{
}

/* HTS_Audio_clear: free audio */
void HTS_Audio_clear(HTS_Audio * audio)
{
}

#endif                          /* AUDIO_PLAY_NONE */

HTS_AUDIO_C_END;

#endif                          /* !HTS_AUDIO_C */
