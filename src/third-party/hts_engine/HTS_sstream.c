/* ----------------------------------------------------------------- */
/*           The HMM-Based Speech Synthesis Engine "hts_engine API"  */
/*           developed by HTS Working Group                          */
/*           http://hts-engine.sourceforge.net/                      */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2001-2015  Nagoya Institute of Technology          */
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

#ifndef HTS_SSTREAM_C
#define HTS_SSTREAM_C

#ifdef __cplusplus
#define HTS_SSTREAM_C_START extern "C" {
#define HTS_SSTREAM_C_END   }
#else
#define HTS_SSTREAM_C_START
#define HTS_SSTREAM_C_END
#endif                          /* __CPLUSPLUS */

HTS_SSTREAM_C_START;

#include <stdlib.h>
#include <math.h>

/* hts_engine libraries */
#include "HTS_hidden.h"

/* HTS_set_default_duration: set default duration from state duration probability distribution */
static double HTS_set_default_duration(size_t * duration, double *mean, double *vari, size_t size)
{
   size_t i;
   double temp;
   size_t sum = 0;

   for (i = 0; i < size; i++) {
      temp = mean[i] + 0.5;
      if (temp < 1.0)
         duration[i] = 1;
      else
         duration[i] = (size_t) temp;
      sum += duration[i];
   }

   return (double) sum;
}

/* HTS_set_specified_duration: set duration from state duration probability distribution and specified frame length */
static double HTS_set_specified_duration(size_t * duration, double *mean, double *vari, size_t size, double frame_length)
{
   size_t i;
   int j;
   double temp1, temp2;
   double rho = 0.0;
   size_t sum = 0;
   size_t target_length;

   /* get the target frame length */
   if (frame_length + 0.5 < 1.0)
      target_length = 1;
   else
      target_length = (size_t) (frame_length + 0.5);

   /* check the specified duration */
   if (target_length <= size) {
      if (target_length < size)
         HTS_error(-1, "HTS_set_specified_duration: Specified frame length is too short.\n");
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
      temp1 = mean[i] + rho * vari[i] + 0.5;
      if (temp1 < 1.0)
         duration[i] = 1;
      else
         duration[i] = (size_t) temp1;
      sum += duration[i];
   }

   /* loop estimation */
   while (target_length != sum) {
      /* sarch flexible state and modify its duration */
      if (target_length > sum) {
         j = -1;
         for (i = 0; i < size; i++) {
            temp2 = fabs(rho - ((double) duration[i] + 1 - mean[i]) / vari[i]);
            if (j < 0 || temp1 > temp2) {
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
               temp2 = fabs(rho - ((double) duration[i] - 1 - mean[i]) / vari[i]);
               if (j < 0 || temp1 > temp2) {
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

/* HTS_SStreamSet_initialize: initialize state stream set */
void HTS_SStreamSet_initialize(HTS_SStreamSet * sss)
{
   sss->nstream = 0;
   sss->nstate = 0;
   sss->sstream = NULL;
   sss->duration = NULL;
   sss->total_state = 0;
   sss->total_frame = 0;
}

/* HTS_SStreamSet_create: parse label and determine state duration */
HTS_Boolean HTS_SStreamSet_create(HTS_SStreamSet * sss, HTS_ModelSet * ms, HTS_Label * label, HTS_Boolean phoneme_alignment_flag, double speed, double *duration_iw, double **parameter_iw, double **gv_iw)
{
   size_t i, j, k;
   double temp;
   int shift;
   size_t state;
   HTS_SStream *sst;
   double *duration_mean, *duration_vari;
   double frame_length;
   size_t next_time;
   size_t next_state;

   if (HTS_Label_get_size(label) == 0)
      return FALSE;

   /* check interpolation weights */
   for (i = 0, temp = 0.0; i < HTS_ModelSet_get_nvoices(ms); i++)
      temp += duration_iw[i];
   if (temp == 0.0) {
      return FALSE;
   } else if (temp != 1.0) {
      for (i = 0; i < HTS_ModelSet_get_nvoices(ms); i++)
         if (duration_iw[i] != 0.0)
            duration_iw[i] /= temp;
   }

   for (i = 0; i < HTS_ModelSet_get_nstream(ms); i++) {
      for (j = 0, temp = 0.0; j < HTS_ModelSet_get_nvoices(ms); j++)
         temp += parameter_iw[j][i];
      if (temp == 0.0) {
         return FALSE;
      } else if (temp != 1.0) {
         for (j = 0; j < HTS_ModelSet_get_nvoices(ms); j++)
            if (parameter_iw[j][i] != 0.0)
               parameter_iw[j][i] /= temp;
      }
      if (HTS_ModelSet_use_gv(ms, i)) {
         for (j = 0, temp = 0.0; j < HTS_ModelSet_get_nvoices(ms); j++)
            temp += gv_iw[j][i];
         if (temp == 0.0)
            return FALSE;
         else if (temp != 1.0)
            for (j = 0; j < HTS_ModelSet_get_nvoices(ms); j++)
               if (gv_iw[j][i] != 0.0)
                  gv_iw[j][i] /= temp;
      }
   }

   /* initialize state sequence */
   sss->nstate = HTS_ModelSet_get_nstate(ms);
   sss->nstream = HTS_ModelSet_get_nstream(ms);
   sss->total_frame = 0;
   sss->total_state = HTS_Label_get_size(label) * sss->nstate;
   sss->duration = (size_t *) HTS_calloc(sss->total_state, sizeof(size_t));
   sss->sstream = (HTS_SStream *) HTS_calloc(sss->nstream, sizeof(HTS_SStream));
   for (i = 0; i < sss->nstream; i++) {
      sst = &sss->sstream[i];
      sst->vector_length = HTS_ModelSet_get_vector_length(ms, i);
      sst->mean = (double **) HTS_calloc(sss->total_state, sizeof(double *));
      sst->vari = (double **) HTS_calloc(sss->total_state, sizeof(double *));
      if (HTS_ModelSet_is_msd(ms, i))
         sst->msd = (double *) HTS_calloc(sss->total_state, sizeof(double));
      else
         sst->msd = NULL;
      for (j = 0; j < sss->total_state; j++) {
         sst->mean[j] = (double *) HTS_calloc(sst->vector_length * HTS_ModelSet_get_window_size(ms, i), sizeof(double));
         sst->vari[j] = (double *) HTS_calloc(sst->vector_length * HTS_ModelSet_get_window_size(ms, i), sizeof(double));
      }
      if (HTS_ModelSet_use_gv(ms, i)) {
         sst->gv_switch = (HTS_Boolean *) HTS_calloc(sss->total_state, sizeof(HTS_Boolean));
         for (j = 0; j < sss->total_state; j++)
            sst->gv_switch[j] = TRUE;
      } else {
         sst->gv_switch = NULL;
      }
   }

   /* determine state duration */
   duration_mean = (double *) HTS_calloc(sss->total_state, sizeof(double));
   duration_vari = (double *) HTS_calloc(sss->total_state, sizeof(double));
   for (i = 0; i < HTS_Label_get_size(label); i++)
      HTS_ModelSet_get_duration(ms, HTS_Label_get_string(label, i), duration_iw, &duration_mean[i * sss->nstate], &duration_vari[i * sss->nstate]);
   if (phoneme_alignment_flag == TRUE) {
      /* use duration set by user */
      next_time = 0;
      next_state = 0;
      state = 0;
      for (i = 0; i < HTS_Label_get_size(label); i++) {
         temp = HTS_Label_get_end_frame(label, i);
         if (temp >= 0) {
            next_time += (size_t) HTS_set_specified_duration(&sss->duration[next_state], &duration_mean[next_state], &duration_vari[next_state], state + sss->nstate - next_state, temp - next_time);
            next_state = state + sss->nstate;
         } else if (i + 1 == HTS_Label_get_size(label)) {
            HTS_error(-1, "HTS_SStreamSet_create: The time of final label is not specified.\n");
            HTS_set_default_duration(&sss->duration[next_state], &duration_mean[next_state], &duration_vari[next_state], state + sss->nstate - next_state);
         }
         state += sss->nstate;
      }
   } else {
      /* determine frame length */
      if (speed != 1.0) {
         temp = 0.0;
         for (i = 0; i < sss->total_state; i++) {
            temp += duration_mean[i];
         }
         frame_length = temp / speed;
         HTS_set_specified_duration(sss->duration, duration_mean, duration_vari, sss->total_state, frame_length);
      } else {
         HTS_set_default_duration(sss->duration, duration_mean, duration_vari, sss->total_state);
      }
   }
   HTS_free(duration_mean);
   HTS_free(duration_vari);

   /* get parameter */
   for (i = 0, state = 0; i < HTS_Label_get_size(label); i++) {
      for (j = 2; j <= sss->nstate + 1; j++) {
         sss->total_frame += sss->duration[state];
         for (k = 0; k < sss->nstream; k++) {
            sst = &sss->sstream[k];
            if (sst->msd)
               HTS_ModelSet_get_parameter(ms, k, j, HTS_Label_get_string(label, i), (const double *const *) parameter_iw, sst->mean[state], sst->vari[state], &sst->msd[state]);
            else
               HTS_ModelSet_get_parameter(ms, k, j, HTS_Label_get_string(label, i), (const double *const *) parameter_iw, sst->mean[state], sst->vari[state], NULL);
         }
         state++;
      }
   }

   /* copy dynamic window */
   for (i = 0; i < sss->nstream; i++) {
      sst = &sss->sstream[i];
      sst->win_size = HTS_ModelSet_get_window_size(ms, i);
      sst->win_max_width = HTS_ModelSet_get_window_max_width(ms, i);
      sst->win_l_width = (int *) HTS_calloc(sst->win_size, sizeof(int));
      sst->win_r_width = (int *) HTS_calloc(sst->win_size, sizeof(int));
      sst->win_coefficient = (double **) HTS_calloc(sst->win_size, sizeof(double));
      for (j = 0; j < sst->win_size; j++) {
         sst->win_l_width[j] = HTS_ModelSet_get_window_left_width(ms, i, j);
         sst->win_r_width[j] = HTS_ModelSet_get_window_right_width(ms, i, j);
         if (sst->win_l_width[j] + sst->win_r_width[j] == 0)
            sst->win_coefficient[j] = (double *) HTS_calloc(-2 * sst->win_l_width[j] + 1, sizeof(double));
         else
            sst->win_coefficient[j] = (double *) HTS_calloc(-2 * sst->win_l_width[j], sizeof(double));
         sst->win_coefficient[j] -= sst->win_l_width[j];
         for (shift = sst->win_l_width[j]; shift <= sst->win_r_width[j]; shift++)
            sst->win_coefficient[j][shift] = HTS_ModelSet_get_window_coefficient(ms, i, j, shift);
      }
   }

   /* determine GV */
   for (i = 0; i < sss->nstream; i++) {
      sst = &sss->sstream[i];
      if (HTS_ModelSet_use_gv(ms, i)) {
         sst->gv_mean = (double *) HTS_calloc(sst->vector_length, sizeof(double));
         sst->gv_vari = (double *) HTS_calloc(sst->vector_length, sizeof(double));
         HTS_ModelSet_get_gv(ms, i, HTS_Label_get_string(label, 0), (const double *const *) gv_iw, sst->gv_mean, sst->gv_vari);
      } else {
         sst->gv_mean = NULL;
         sst->gv_vari = NULL;
      }
   }

   for (i = 0; i < HTS_Label_get_size(label); i++)
      if (HTS_ModelSet_get_gv_flag(ms, HTS_Label_get_string(label, i)) == FALSE)
         for (j = 0; j < sss->nstream; j++)
            if (HTS_ModelSet_use_gv(ms, j) == TRUE)
               for (k = 0; k < sss->nstate; k++)
                  sss->sstream[j].gv_switch[i * sss->nstate + k] = FALSE;

   return TRUE;
}

/* HTS_SStreamSet_get_nstream: get number of stream */
size_t HTS_SStreamSet_get_nstream(HTS_SStreamSet * sss)
{
   return sss->nstream;
}

/* HTS_SStreamSet_get_vector_length: get vector length */
size_t HTS_SStreamSet_get_vector_length(HTS_SStreamSet * sss, size_t stream_index)
{
   return sss->sstream[stream_index].vector_length;
}

/* HTS_SStreamSet_is_msd: get MSD flag */
HTS_Boolean HTS_SStreamSet_is_msd(HTS_SStreamSet * sss, size_t stream_index)
{
   return sss->sstream[stream_index].msd ? TRUE : FALSE;
}

/* HTS_SStreamSet_get_total_state: get total number of state */
size_t HTS_SStreamSet_get_total_state(HTS_SStreamSet * sss)
{
   return sss->total_state;
}

/* HTS_SStreamSet_get_total_frame: get total number of frame */
size_t HTS_SStreamSet_get_total_frame(HTS_SStreamSet * sss)
{
   return sss->total_frame;
}

/* HTS_SStreamSet_get_msd: get MSD parameter */
double HTS_SStreamSet_get_msd(HTS_SStreamSet * sss, size_t stream_index, size_t state_index)
{
   return sss->sstream[stream_index].msd[state_index];
}

/* HTS_SStreamSet_window_size: get dynamic window size */
size_t HTS_SStreamSet_get_window_size(HTS_SStreamSet * sss, size_t stream_index)
{
   return sss->sstream[stream_index].win_size;
}

/* HTS_SStreamSet_get_window_left_width: get left width of dynamic window */
int HTS_SStreamSet_get_window_left_width(HTS_SStreamSet * sss, size_t stream_index, size_t window_index)
{
   return sss->sstream[stream_index].win_l_width[window_index];
}

/* HTS_SStreamSet_get_winodow_right_width: get right width of dynamic window */
int HTS_SStreamSet_get_window_right_width(HTS_SStreamSet * sss, size_t stream_index, size_t window_index)
{
   return sss->sstream[stream_index].win_r_width[window_index];
}

/* HTS_SStreamSet_get_window_coefficient: get coefficient of dynamic window */
double HTS_SStreamSet_get_window_coefficient(HTS_SStreamSet * sss, size_t stream_index, size_t window_index, int coefficient_index)
{
   return sss->sstream[stream_index].win_coefficient[window_index][coefficient_index];
}

/* HTS_SStreamSet_get_window_max_width: get max width of dynamic window */
size_t HTS_SStreamSet_get_window_max_width(HTS_SStreamSet * sss, size_t stream_index)
{
   return sss->sstream[stream_index].win_max_width;
}

/* HTS_SStreamSet_use_gv: get GV flag */
HTS_Boolean HTS_SStreamSet_use_gv(HTS_SStreamSet * sss, size_t stream_index)
{
   return sss->sstream[stream_index].gv_mean ? TRUE : FALSE;
}

/* HTS_SStreamSet_get_duration: get state duration */
size_t HTS_SStreamSet_get_duration(HTS_SStreamSet * sss, size_t state_index)
{
   return sss->duration[state_index];
}

/* HTS_SStreamSet_get_mean: get mean parameter */
double HTS_SStreamSet_get_mean(HTS_SStreamSet * sss, size_t stream_index, size_t state_index, size_t vector_index)
{
   return sss->sstream[stream_index].mean[state_index][vector_index];
}

/* HTS_SStreamSet_set_mean: set mean parameter */
void HTS_SStreamSet_set_mean(HTS_SStreamSet * sss, size_t stream_index, size_t state_index, size_t vector_index, double f)
{
   sss->sstream[stream_index].mean[state_index][vector_index] = f;
}

/* HTS_SStreamSet_get_vari: get variance parameter */
double HTS_SStreamSet_get_vari(HTS_SStreamSet * sss, size_t stream_index, size_t state_index, size_t vector_index)
{
   return sss->sstream[stream_index].vari[state_index][vector_index];
}

/* HTS_SStreamSet_set_vari: set variance parameter */
void HTS_SStreamSet_set_vari(HTS_SStreamSet * sss, size_t stream_index, size_t state_index, size_t vector_index, double f)
{
   sss->sstream[stream_index].vari[state_index][vector_index] = f;
}

/* HTS_SStreamSet_get_gv_mean: get GV mean parameter */
double HTS_SStreamSet_get_gv_mean(HTS_SStreamSet * sss, size_t stream_index, size_t vector_index)
{
   return sss->sstream[stream_index].gv_mean[vector_index];
}

/* HTS_SStreamSet_get_gv_mean: get GV variance parameter */
double HTS_SStreamSet_get_gv_vari(HTS_SStreamSet * sss, size_t stream_index, size_t vector_index)
{
   return sss->sstream[stream_index].gv_vari[vector_index];
}

/* HTS_SStreamSet_set_gv_switch: set GV switch */
void HTS_SStreamSet_set_gv_switch(HTS_SStreamSet * sss, size_t stream_index, size_t state_index, HTS_Boolean i)
{
   sss->sstream[stream_index].gv_switch[state_index] = i;
}

/* HTS_SStreamSet_get_gv_switch: get GV switch */
HTS_Boolean HTS_SStreamSet_get_gv_switch(HTS_SStreamSet * sss, size_t stream_index, size_t state_index)
{
   return sss->sstream[stream_index].gv_switch[state_index];
}

/* HTS_SStreamSet_clear: free state stream set */
void HTS_SStreamSet_clear(HTS_SStreamSet * sss)
{
   size_t i, j;
   HTS_SStream *sst;

   if (sss->sstream) {
      for (i = 0; i < sss->nstream; i++) {
         sst = &sss->sstream[i];
         for (j = 0; j < sss->total_state; j++) {
            HTS_free(sst->mean[j]);
            HTS_free(sst->vari[j]);
         }
         if (sst->msd)
            HTS_free(sst->msd);
         HTS_free(sst->mean);
         HTS_free(sst->vari);
         for (j = 0; j < sst->win_size; j++) {
            sst->win_coefficient[j] += sst->win_l_width[j];
            HTS_free(sst->win_coefficient[j]);
         }
         HTS_free(sst->win_coefficient);
         HTS_free(sst->win_l_width);
         HTS_free(sst->win_r_width);
         if (sst->gv_mean)
            HTS_free(sst->gv_mean);
         if (sst->gv_vari)
            HTS_free(sst->gv_vari);
         if (sst->gv_switch)
            HTS_free(sst->gv_switch);
      }
      HTS_free(sss->sstream);
   }
   if (sss->duration)
      HTS_free(sss->duration);

   HTS_SStreamSet_initialize(sss);
}

HTS_SSTREAM_C_END;

#endif                          /* !HTS_SSTREAM_C */
