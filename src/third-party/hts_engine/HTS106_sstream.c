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

#ifndef HTS106_SSTREAM_C
#define HTS106_SSTREAM_C

#ifdef __cplusplus
#define HTS106_SSTREAM_C_START extern "C" {
#define HTS106_SSTREAM_C_END   }
#else
#define HTS106_SSTREAM_C_START
#define HTS106_SSTREAM_C_END
#endif                          /* __CPLUSPLUS */

HTS106_SSTREAM_C_START;

#include <stdlib.h>

/* hts_engine libraries */
#include "HTS106_hidden.h"

/* HTS106_set_duration: set duration from state duration probability distribution */
static double HTS106_set_duration(int *duration, double *mean, double *vari, int size, double frame_length)
{
   int i, j;
   double temp1, temp2;
   double rho = 0.0;
   int sum = 0;
   int target_length;

   /* if the frame length is not specified, only the mean vector is used */
   if (frame_length == 0.0) {
      for (i = 0; i < size; i++) {
         duration[i] = (int) (mean[i] + 0.5);
         if (duration[i] < 1)
            duration[i] = 1;
         sum += duration[i];
      }
      return (double) sum;
   }

   /* get the target frame length */
   target_length = (int) (frame_length + 0.5);

   /* check the specified duration */
   if (target_length <= size) {
      if (target_length < size)
         HTS106_error(-1, "HTS106_set_duration: Specified frame length is too short.\n");
      for (i = 0; i < size; i++)
         duration[i] = 1;
      return (double) size;
   }

   /* RHO calculation */
   temp1 = 0.0;
   temp2 = 0.0;
   for (i = 0; i < size; i++) {
      temp1 += mean[i];
      temp2 += vari[i];
   }
   rho = ((double) target_length - temp1) / temp2;

   /* first estimation */
   for (i = 0; i < size; i++) {
      duration[i] = (int) (mean[i] + rho * vari[i] + 0.5);
      if (duration[i] < 1)
         duration[i] = 1;
      sum += duration[i];
   }

   /* loop estimation */
   while (target_length != sum) {
      /* sarch flexible state and modify its duration */
      if (target_length > sum) {
         j = -1;
         for (i = 0; i < size; i++) {
            temp2 = abs(rho - ((double) duration[i] + 1 - mean[i]) / vari[i]);
            if (j < 0 || temp1 < temp2) {
               j = i;
               temp1 = temp2;
            }
         }
         sum++;
         duration[j]++;
      } else {
         j = -1;
         for (i = 0; i < size; i++) {
            if (duration[i] > 1) {
               temp2 = abs(rho - ((double) duration[i] - 1 - mean[i]) / vari[i]);
               if (j < 0 || temp1 < temp2) {
                  j = i;
                  temp1 = temp2;
               }
            }
         }
         sum--;
         duration[j]--;
      }
   }

   return (double) target_length;
}

/* HTS106_SStreamSet_initialize: initialize state stream set */
void HTS106_SStreamSet_initialize(HTS106_SStreamSet * sss)
{
   sss->nstream = 0;
   sss->nstate = 0;
   sss->sstream = NULL;
   sss->duration = NULL;
   sss->total_state = 0;
   sss->total_frame = 0;
}

/* HTS106_SStreamSet_create: parse label and determine state duration */
HTS106_Boolean HTS106_SStreamSet_create(HTS106_SStreamSet * sss, HTS106_ModelSet * ms, HTS106_Label * label, double *duration_iw, double **parameter_iw, double **gv_iw)
{
   int i, j, k;
   double temp;
   int state;
   HTS106_SStream *sst;
   double *duration_mean, *duration_vari;
   double frame_length;
   int next_time;
   int next_state;

   /* check interpolation weights */
   for (i = 0, temp = 0.0; i < HTS106_ModelSet_get_duration_interpolation_size(ms); i++)
      temp += duration_iw[i];
   if (temp == 0.0)
      return FALSE;
   for (i = 0; i < sss->nstream; i++) {
      for (j = 0, temp = 0.0; j < HTS106_ModelSet_get_parameter_interpolation_size(ms, i); j++)
         temp += parameter_iw[i][j];
      if (temp == 0.0)
         return FALSE;
      if (HTS106_ModelSet_use_gv(ms, i)) {
         for (j = 0, temp = 0.0; j < HTS106_ModelSet_get_gv_interpolation_size(ms, i); j++)
            temp += gv_iw[i][j];
         return FALSE;
      }
   }

   /* initialize state sequence */
   sss->nstate = HTS106_ModelSet_get_nstate(ms);
   sss->nstream = HTS106_ModelSet_get_nstream(ms);
   sss->total_frame = 0;
   sss->total_state = HTS106_Label_get_size(label) * sss->nstate;
   sss->duration = (int *) HTS106_calloc(sss->total_state, sizeof(int));
   sss->sstream = (HTS106_SStream *) HTS106_calloc(sss->nstream, sizeof(HTS106_SStream));
   for (i = 0; i < sss->nstream; i++) {
      sst = &sss->sstream[i];
      sst->vector_length = HTS106_ModelSet_get_vector_length(ms, i);
      sst->mean = (double **) HTS106_calloc(sss->total_state, sizeof(double *));
      sst->vari = (double **) HTS106_calloc(sss->total_state, sizeof(double *));
      if (HTS106_ModelSet_is_msd(ms, i))
         sst->msd = (double *) HTS106_calloc(sss->total_state, sizeof(double));
      else
         sst->msd = NULL;
      for (j = 0; j < sss->total_state; j++) {
         sst->mean[j] = (double *) HTS106_calloc(sst->vector_length, sizeof(double));
         sst->vari[j] = (double *) HTS106_calloc(sst->vector_length, sizeof(double));
      }
      sst->gv_switch = (HTS106_Boolean *) HTS106_calloc(sss->total_state, sizeof(HTS106_Boolean));
      for (j = 0; j < sss->total_state; j++)
         sst->gv_switch[j] = TRUE;
   }

   /* check interpolation weights */
   for (i = 0, temp = 0.0; i < HTS106_ModelSet_get_duration_interpolation_size(ms); i++)
      temp += duration_iw[i];
   for (i = 0; i < HTS106_ModelSet_get_duration_interpolation_size(ms); i++)
      if (duration_iw[i] != 0.0)
         duration_iw[i] /= temp;
   for (i = 0; i < sss->nstream; i++) {
      for (j = 0, temp = 0.0; j < HTS106_ModelSet_get_parameter_interpolation_size(ms, i); j++)
         temp += parameter_iw[i][j];
      for (j = 0; j < HTS106_ModelSet_get_parameter_interpolation_size(ms, i); j++)
         if (parameter_iw[i][j] != 0.0)
            parameter_iw[i][j] /= temp;
      if (HTS106_ModelSet_use_gv(ms, i)) {
         for (j = 0, temp = 0.0; j < HTS106_ModelSet_get_gv_interpolation_size(ms, i); j++)
            temp += gv_iw[i][j];
         for (j = 0; j < HTS106_ModelSet_get_gv_interpolation_size(ms, i); j++)
            if (gv_iw[i][j] != 0.0)
               gv_iw[i][j] /= temp;
      }
   }

   /* determine state duration */
   duration_mean = (double *) HTS106_calloc(sss->nstate * HTS106_Label_get_size(label), sizeof(double));
   duration_vari = (double *) HTS106_calloc(sss->nstate * HTS106_Label_get_size(label), sizeof(double));
   for (i = 0; i < HTS106_Label_get_size(label); i++)
     HTS106_ModelSet_get_duration(ms, HTS106_Label_get_string(label, i), NULL, &duration_mean[i * sss->nstate], &duration_vari[i * sss->nstate], duration_iw);
   if (HTS106_Label_get_frame_specified_flag(label)) {
      /* use duration set by user */
      next_time = 0;
      next_state = 0;
      state = 0;
      for (i = 0; i < HTS106_Label_get_size(label); i++) {
         temp = HTS106_Label_get_end_frame(label, i);
         if (temp >= 0) {
            next_time += HTS106_set_duration(&sss->duration[next_state], &duration_mean[next_state], &duration_vari[next_state], state + sss->nstate - next_state, temp - next_time);
            next_state = state + sss->nstate;
         } else if (i + 1 == HTS106_Label_get_size(label)) {
            HTS106_error(-1, "HTS106_SStreamSet_create: The time of final label is not specified.\n");
            HTS106_set_duration(&sss->duration[next_state], &duration_mean[next_state], &duration_vari[next_state], state + sss->nstate - next_state, 0.0);
         }
         state += sss->nstate;
      }
   } else {
      /* determine frame length */
      if (HTS106_Label_get_speech_speed(label) != 1.0) {
         temp = 0.0;
         for (i = 0; i < HTS106_Label_get_size(label) * sss->nstate; i++) {
            temp += duration_mean[i];
         }
         frame_length = temp / HTS106_Label_get_speech_speed(label);
      } else {
         frame_length = 0.0;
      }
      /* set state duration */
      HTS106_set_duration(sss->duration, duration_mean, duration_vari, HTS106_Label_get_size(label) * sss->nstate, frame_length);
   }
   HTS106_free(duration_mean);
   HTS106_free(duration_vari);

   /* get parameter */
   for (i = 0, state = 0; i < HTS106_Label_get_size(label); i++) {
      for (j = 2; j <= sss->nstate + 1; j++) {
         sss->total_frame += sss->duration[state];
         for (k = 0; k < sss->nstream; k++) {
            sst = &sss->sstream[k];
            if (sst->msd)
              HTS106_ModelSet_get_parameter(ms, HTS106_Label_get_string(label, i), NULL, sst->mean[state], sst->vari[state], &sst->msd[state], k, j, parameter_iw[k]);
            else
              HTS106_ModelSet_get_parameter(ms, HTS106_Label_get_string(label, i), NULL, sst->mean[state], sst->vari[state], NULL, k, j, parameter_iw[k]);
         }
         state++;
      }
   }

   /* copy dynamic window */
   for (i = 0; i < sss->nstream; i++) {
      sst = &sss->sstream[i];
      sst->win_size = HTS106_ModelSet_get_window_size(ms, i);
      sst->win_max_width = HTS106_ModelSet_get_window_max_width(ms, i);
      sst->win_l_width = (int *) HTS106_calloc(sst->win_size, sizeof(int));
      sst->win_r_width = (int *) HTS106_calloc(sst->win_size, sizeof(int));
      sst->win_coefficient = (double **) HTS106_calloc(sst->win_size, sizeof(double));
      for (j = 0; j < sst->win_size; j++) {
         sst->win_l_width[j] = HTS106_ModelSet_get_window_left_width(ms, i, j);
         sst->win_r_width[j] = HTS106_ModelSet_get_window_right_width(ms, i, j);
         if (sst->win_l_width[j] + sst->win_r_width[j] == 0)
            sst->win_coefficient[j] = (double *) HTS106_calloc(-2 * sst->win_l_width[j] + 1, sizeof(double));
         else
            sst->win_coefficient[j] = (double *) HTS106_calloc(-2 * sst->win_l_width[j], sizeof(double));
         sst->win_coefficient[j] -= sst->win_l_width[j];
         for (k = sst->win_l_width[j]; k <= sst->win_r_width[j]; k++)
            sst->win_coefficient[j][k] = HTS106_ModelSet_get_window_coefficient(ms, i, j, k);
      }
   }

   /* determine GV */
   for (i = 0; i < sss->nstream; i++) {
      sst = &sss->sstream[i];
      if (HTS106_ModelSet_use_gv(ms, i)) {
         sst->gv_mean = (double *) HTS106_calloc(sst->vector_length / sst->win_size, sizeof(double));
         sst->gv_vari = (double *) HTS106_calloc(sst->vector_length / sst->win_size, sizeof(double));
         HTS106_ModelSet_get_gv(ms, HTS106_Label_get_string(label, 0), sst->gv_mean, sst->gv_vari, i, gv_iw[i]);
      } else {
         sst->gv_mean = NULL;
         sst->gv_vari = NULL;
      }
   }

   if (HTS106_ModelSet_have_gv_switch(ms) == TRUE)
      for (i = 0; i < HTS106_Label_get_size(label); i++)
         if (HTS106_ModelSet_get_gv_switch(ms, HTS106_Label_get_string(label, i)) == FALSE)
            for (j = 0; j < sss->nstream; j++)
               for (k = 0; k < sss->nstate; k++)
                  sss->sstream[j].gv_switch[i * sss->nstate + k] = FALSE;

   return TRUE;
}

/* HTS106_SStreamSet_get_nstream: get number of stream */
int HTS106_SStreamSet_get_nstream(HTS106_SStreamSet * sss)
{
   return sss->nstream;
}

/* HTS106_SStreamSet_get_vector_length: get vector length */
int HTS106_SStreamSet_get_vector_length(HTS106_SStreamSet * sss, int stream_index)
{
   return sss->sstream[stream_index].vector_length;
}

/* HTS106_SStreamSet_is_msd: get MSD flag */
HTS106_Boolean HTS106_SStreamSet_is_msd(HTS106_SStreamSet * sss, int stream_index)
{
   return sss->sstream[stream_index].msd ? TRUE : FALSE;
}

/* HTS106_SStreamSet_get_total_state: get total number of state */
int HTS106_SStreamSet_get_total_state(HTS106_SStreamSet * sss)
{
   return sss->total_state;
}

/* HTS106_SStreamSet_get_total_frame: get total number of frame */
int HTS106_SStreamSet_get_total_frame(HTS106_SStreamSet * sss)
{
   return sss->total_frame;
}

/* HTS106_SStreamSet_get_msd: get MSD parameter */
double HTS106_SStreamSet_get_msd(HTS106_SStreamSet * sss, int stream_index, int state_index)
{
   return sss->sstream[stream_index].msd[state_index];
}

/* HTS106_SStreamSet_window_size: get dynamic window size */
int HTS106_SStreamSet_get_window_size(HTS106_SStreamSet * sss, int stream_index)
{
   return sss->sstream[stream_index].win_size;
}

/* HTS106_SStreamSet_get_window_left_width: get left width of dynamic window */
int HTS106_SStreamSet_get_window_left_width(HTS106_SStreamSet * sss, int stream_index, int window_index)
{
   return sss->sstream[stream_index].win_l_width[window_index];
}

/* HTS106_SStreamSet_get_winodow_right_width: get right width of dynamic window */
int HTS106_SStreamSet_get_window_right_width(HTS106_SStreamSet * sss, int stream_index, int window_index)
{
   return sss->sstream[stream_index].win_r_width[window_index];
}

/* HTS106_SStreamSet_get_window_coefficient: get coefficient of dynamic window */
double HTS106_SStreamSet_get_window_coefficient(HTS106_SStreamSet * sss, int stream_index, int window_index, int coefficient_index)
{
   return sss->sstream[stream_index].win_coefficient[window_index][coefficient_index];
}

/* HTS106_SStreamSet_get_window_max_width: get max width of dynamic window */
int HTS106_SStreamSet_get_window_max_width(HTS106_SStreamSet * sss, int stream_index)
{
   return sss->sstream[stream_index].win_max_width;
}

/* HTS106_SStreamSet_use_gv: get GV flag */
HTS106_Boolean HTS106_SStreamSet_use_gv(HTS106_SStreamSet * sss, int stream_index)
{
   return sss->sstream[stream_index].gv_mean ? TRUE : FALSE;
}

/* HTS106_SStreamSet_get_duration: get state duration */
int HTS106_SStreamSet_get_duration(HTS106_SStreamSet * sss, int state_index)
{
   return sss->duration[state_index];
}

/* HTS106_SStreamSet_get_mean: get mean parameter */
double HTS106_SStreamSet_get_mean(HTS106_SStreamSet * sss, int stream_index, int state_index, int vector_index)
{
   return sss->sstream[stream_index].mean[state_index][vector_index];
}

/* HTS106_SStreamSet_set_mean: set mean parameter */
void HTS106_SStreamSet_set_mean(HTS106_SStreamSet * sss, int stream_index, int state_index, int vector_index, double f)
{
   sss->sstream[stream_index].mean[state_index][vector_index] = f;
}

/* HTS106_SStreamSet_get_vari: get variance parameter */
double HTS106_SStreamSet_get_vari(HTS106_SStreamSet * sss, int stream_index, int state_index, int vector_index)
{
   return sss->sstream[stream_index].vari[state_index][vector_index];
}

/* HTS106_SStreamSet_set_vari: set variance parameter */
void HTS106_SStreamSet_set_vari(HTS106_SStreamSet * sss, int stream_index, int state_index, int vector_index, double f)
{
   sss->sstream[stream_index].vari[state_index][vector_index] = f;
}

/* HTS106_SStreamSet_get_gv_mean: get GV mean parameter */
double HTS106_SStreamSet_get_gv_mean(HTS106_SStreamSet * sss, int stream_index, int vector_index)
{
   return sss->sstream[stream_index].gv_mean[vector_index];
}

/* HTS106_SStreamSet_get_gv_mean: get GV variance parameter */
double HTS106_SStreamSet_get_gv_vari(HTS106_SStreamSet * sss, int stream_index, int vector_index)
{
   return sss->sstream[stream_index].gv_vari[vector_index];
}

/* HTS106_SStreamSet_set_gv_switch: set GV switch */
void HTS106_SStreamSet_set_gv_switch(HTS106_SStreamSet * sss, int stream_index, int state_index, HTS106_Boolean i)
{
   sss->sstream[stream_index].gv_switch[state_index] = i;
}

/* HTS106_SStreamSet_get_gv_switch: get GV switch */
HTS106_Boolean HTS106_SStreamSet_get_gv_switch(HTS106_SStreamSet * sss, int stream_index, int state_index)
{
   return sss->sstream[stream_index].gv_switch[state_index];
}

/* HTS106_SStreamSet_clear: free state stream set */
void HTS106_SStreamSet_clear(HTS106_SStreamSet * sss)
{
   int i, j;
   HTS106_SStream *sst;

   if (sss->sstream) {
      for (i = 0; i < sss->nstream; i++) {
         sst = &sss->sstream[i];
         for (j = 0; j < sss->total_state; j++) {
            HTS106_free(sst->mean[j]);
            HTS106_free(sst->vari[j]);
         }
         if (sst->msd)
            HTS106_free(sst->msd);
         HTS106_free(sst->mean);
         HTS106_free(sst->vari);
         for (j = sst->win_size - 1; j >= 0; j--) {
            sst->win_coefficient[j] += sst->win_l_width[j];
            HTS106_free(sst->win_coefficient[j]);
         }
         HTS106_free(sst->win_coefficient);
         HTS106_free(sst->win_l_width);
         HTS106_free(sst->win_r_width);
         if (sst->gv_mean)
            HTS106_free(sst->gv_mean);
         if (sst->gv_vari)
            HTS106_free(sst->gv_vari);
         HTS106_free(sst->gv_switch);
      }
      HTS106_free(sss->sstream);
   }
   if (sss->duration)
      HTS106_free(sss->duration);

   HTS106_SStreamSet_initialize(sss);
}

HTS106_SSTREAM_C_END;

#endif                          /* !HTS106_SSTREAM_C */
