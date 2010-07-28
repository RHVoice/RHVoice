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

/* hts_engine libraries */
#include "HTS_hidden.h"

/* HTS_GStreamSet_initialize: initialize generated parameter stream set */
void HTS_GStreamSet_initialize(HTS_GStreamSet * gss)
{
   gss->nstream = 0;
   gss->total_frame = 0;
   gss->total_nsample = 0;
#ifndef HTS_EMBEDDED
   gss->gstream = NULL;
#endif                          /* !HTS_EMBEDDED */
   gss->gspeech = NULL;
}

/* HTS_GStreamSet_create: generate speech */
/* (stream[0] == spectrum && stream[1] == lf0) */
void HTS_GStreamSet_create(HTS_GStreamSet * gss, HTS_PStreamSet * pss,
                           int stage, HTS_Boolean use_log_gain,
                           int sampling_rate, int fperiod, double alpha,
                           double beta, int audio_buff_size,
                           HTS_SynthCallback callback, void *user_data)
{
   int i, j, k;
#ifdef HTS_EMBEDDED
   double lf0;
#endif                          /* HTS_EMBEDDED */
   int cont_synth=1;
   int msd_frame;
   HTS_Vocoder v;

   /* check */
#ifdef HTS_EMBEDDED
   if (gss->gspeech)
#else
   if (gss->gstream || gss->gspeech)
#endif                          /* HTS_EMBEDDED */
      HTS_error(1,
                "HTS_GStreamSet_create: HTS_GStreamSet is not initialized.\n");

   /* initialize */
   gss->nstream = HTS_PStreamSet_get_nstream(pss);
   gss->total_frame = HTS_PStreamSet_get_total_frame(pss);
   gss->total_nsample = fperiod * gss->total_frame;
#ifndef HTS_EMBEDDED
   gss->gstream = (HTS_GStream *) HTS_calloc(gss->nstream, sizeof(HTS_GStream));
   for (i = 0; i < gss->nstream; i++) {
      gss->gstream[i].static_length = HTS_PStreamSet_get_static_length(pss, i);
      gss->gstream[i].par =
          (double **) HTS_calloc(gss->total_frame, sizeof(double *));
      for (j = 0; j < gss->total_frame; j++)
         gss->gstream[i].par[j] =
             (double *) HTS_calloc(gss->gstream[i].static_length,
                                   sizeof(double));
   }
#endif                          /* !HTS_EMBEDDED */
   gss->gspeech = (short *) HTS_calloc(gss->total_nsample, sizeof(short));

#ifndef HTS_EMBEDDED
   /* copy generated parameter */
   for (i = 0; i < gss->nstream; i++) {
      if (HTS_PStreamSet_is_msd(pss, i)) {      /* for MSD */
         for (j = 0, msd_frame = 0; j < gss->total_frame; j++)
            if (HTS_PStreamSet_get_msd_flag(pss, i, j)) {
               for (k = 0; k < gss->gstream[i].static_length; k++)
                  gss->gstream[i].par[j][k] =
                      HTS_PStreamSet_get_parameter(pss, i, msd_frame, k);
               msd_frame++;
            } else
               for (k = 0; k < gss->gstream[i].static_length; k++)
                  gss->gstream[i].par[j][k] = LZERO;
      } else {                  /* for non MSD */
         for (j = 0; j < gss->total_frame; j++)
            for (k = 0; k < gss->gstream[i].static_length; k++)
               gss->gstream[i].par[j][k] =
                   HTS_PStreamSet_get_parameter(pss, i, j, k);
      }
   }
#endif                          /* !HTS_EMBEDDED */

   /* check */
   if (gss->nstream != 2)
      HTS_error(1,
                "HTS_GStreamSet_create: The number of streams should be 2.\n");
   if (HTS_PStreamSet_get_static_length(pss, 1) != 1)
      HTS_error(1,
                "HTS_GStreamSet_create: The size of lf0 static vector should be 1.\n");

   /* synthesize speech waveform */
#ifdef HTS_EMBEDDED
   HTS_Vocoder_initialize(&v, HTS_PStreamSet_get_static_length(pss, 0) - 1,
                          stage, use_log_gain, sampling_rate, fperiod,
                          audio_buff_size);
   for (i = 0, msd_frame = 0; cont_synth&&(i < gss->total_frame); i++) {
      lf0 = LZERO;
      if (HTS_PStreamSet_get_msd_flag(pss, 1, i))
         lf0 = HTS_PStreamSet_get_parameter(pss, 1, msd_frame++, 0);
      HTS_Vocoder_synthesize(&v, HTS_PStreamSet_get_static_length(pss, 0) - 1,
                             lf0,
                             HTS_PStreamSet_get_parameter_vector(pss, 0, i),
                             alpha, beta, &gss->gspeech[i * fperiod]);
      if(callback)
        cont_synth=callback(&gss->gspeech[i * fperiod], fperiod, user_data);
   }
#else
   HTS_Vocoder_initialize(&v, gss->gstream[0].static_length - 1, stage,
                          use_log_gain, sampling_rate, fperiod,
                          audio_buff_size);
   for (i = 0; cont_synth&&(i < gss->total_frame); i++) {
      HTS_Vocoder_synthesize(&v, gss->gstream[0].static_length - 1,
                             gss->gstream[1].par[i][0],
                             &gss->gstream[0].par[i][0], alpha, beta,
                             &gss->gspeech[i * fperiod]);
      if(callback)
        cont_synth=callback(&gss->gspeech[i * fperiod], fperiod, user_data);
   }
#endif                          /* HTS_EMBEDDED */
   HTS_Vocoder_clear(&v);
}

/* HTS_GStreamSet_get_total_nsample: get total number of sample */
int HTS_GStreamSet_get_total_nsample(HTS_GStreamSet * gss)
{
   return gss->total_nsample;
}

/* HTS_GStreamSet_get_total_frame: get total number of frame */
int HTS_GStreamSet_get_total_frame(HTS_GStreamSet * gss)
{
   return gss->total_frame;
}

#ifndef HTS_EMBEDDED
/* HTS_GStreamSet_get_static_length: get static features length */
int HTS_GStreamSet_get_static_length(HTS_GStreamSet * gss, int stream_index)
{
   return gss->gstream[stream_index].static_length;
}
#endif                          /* !HTS_EMBEDDED */

/* HTS_GStreamSet_get_speech: get synthesized speech parameter */
short HTS_GStreamSet_get_speech(HTS_GStreamSet * gss, int sample_index)
{
   return gss->gspeech[sample_index];
}

#ifndef HTS_EMBEDDED
/* HTS_GStreamSet_get_parameter: get generated parameter */
double HTS_GStreamSet_get_parameter(HTS_GStreamSet * gss, int stream_index,
                                    int frame_index, int vector_index)
{
   return gss->gstream[stream_index].par[frame_index][vector_index];
}
#endif                          /* !HTS_EMBEDDED */

/* HTS_GStreamSet_clear: free generated parameter stream set */
void HTS_GStreamSet_clear(HTS_GStreamSet * gss)
{
   int i, j;

#ifndef HTS_EMBEDDED
   if (gss->gstream) {
      for (i = 0; i < gss->nstream; i++) {
         for (j = 0; j < gss->total_frame; j++)
            HTS_free(gss->gstream[i].par[j]);
         HTS_free(gss->gstream[i].par);
      }
      HTS_free(gss->gstream);
   }
#endif                          /* !HTS_EMBEDDED */
   if (gss->gspeech)
      HTS_free(gss->gspeech);
   HTS_GStreamSet_initialize(gss);
}
