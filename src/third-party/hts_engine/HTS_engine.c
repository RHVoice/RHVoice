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

#ifndef HTS_ENGINE_C
#define HTS_ENGINE_C

#ifdef __cplusplus
#define HTS_ENGINE_C_START extern "C" {
#define HTS_ENGINE_C_END   }
#else
#define HTS_ENGINE_C_START
#define HTS_ENGINE_C_END
#endif                          /* __CPLUSPLUS */

HTS_ENGINE_C_START;

#include <stdlib.h>             /* for atof() */
#include <string.h>             /* for strcpy() */
#include <math.h>               /* for pow() */

/* hts_engine libraries */
#include "HTS_hidden.h"

/* HTS_Engine_initialize: initialize engine */
void HTS_Engine_initialize(HTS_Engine * engine)
{
   /* global */
   engine->condition.sampling_frequency = 0;
   engine->condition.fperiod = 0;
   engine->condition.audio_buff_size = 0;
   engine->condition.stop = FALSE;
   engine->condition.volume = 1.0;
   engine->condition.msd_threshold = NULL;
   engine->condition.gv_weight = NULL;

   /* duration */
   engine->condition.speed = 1.0;
   engine->condition.phoneme_alignment_flag = FALSE;

   /* spectrum */
   engine->condition.stage = 0;
   engine->condition.use_log_gain = FALSE;
   engine->condition.alpha = 0.0;
   engine->condition.beta = 0.0;

   /* log F0 */
   engine->condition.additional_half_tone = 0.0;

   /* interpolation weights */
   engine->condition.duration_iw = NULL;
   engine->condition.parameter_iw = NULL;
   engine->condition.gv_iw = NULL;

   /* initialize audio */
   HTS_Audio_initialize(&engine->audio);
   /* initialize model set */
   HTS_ModelSet_initialize(&engine->ms);
   /* initialize label list */
   HTS_Label_initialize(&engine->label);
   /* initialize state sequence set */
   HTS_SStreamSet_initialize(&engine->sss);
   /* initialize pstream set */
   HTS_PStreamSet_initialize(&engine->pss);
   /* initialize gstream set */
   HTS_GStreamSet_initialize(&engine->gss);
}

/* HTS_Engine_load: load HTS voices */
HTS_Boolean HTS_Engine_load(HTS_Engine * engine, char **voices, size_t num_voices)
{
   size_t i, j;
   size_t nstream;
   double average_weight;
   const char *option, *find;

   /* reset engine */
   HTS_Engine_clear(engine);

   /* load voices */
   if (HTS_ModelSet_load(&engine->ms, voices, num_voices) != TRUE) {
      HTS_Engine_clear(engine);
      return FALSE;
   }
   nstream = HTS_ModelSet_get_nstream(&engine->ms);
   average_weight = 1.0 / num_voices;

   /* global */
   engine->condition.sampling_frequency = HTS_ModelSet_get_sampling_frequency(&engine->ms);
   engine->condition.fperiod = HTS_ModelSet_get_fperiod(&engine->ms);
   engine->condition.msd_threshold = (double *) HTS_calloc(nstream, sizeof(double));
   for (i = 0; i < nstream; i++)
      engine->condition.msd_threshold[i] = 0.5;
   engine->condition.gv_weight = (double *) HTS_calloc(nstream, sizeof(double));
   for (i = 0; i < nstream; i++)
      engine->condition.gv_weight[i] = 1.0;

   /* spectrum */
   option = HTS_ModelSet_get_option(&engine->ms, 0);
   find = strstr(option, "GAMMA=");
   if (find != NULL)
      engine->condition.stage = (size_t) atoi(&find[strlen("GAMMA=")]);
   find = strstr(option, "LN_GAIN=");
   if (find != NULL)
      engine->condition.use_log_gain = atoi(&find[strlen("LN_GAIN=")]) == 1 ? TRUE : FALSE;
   find = strstr(option, "ALPHA=");
   if (find != NULL)
      engine->condition.alpha = atof(&find[strlen("ALPHA=")]);

   /* interpolation weights */
   engine->condition.duration_iw = (double *) HTS_calloc(num_voices, sizeof(double));
   for (i = 0; i < num_voices; i++)
      engine->condition.duration_iw[i] = average_weight;
   engine->condition.parameter_iw = (double **) HTS_calloc(num_voices, sizeof(double *));
   for (i = 0; i < num_voices; i++) {
      engine->condition.parameter_iw[i] = (double *) HTS_calloc(nstream, sizeof(double));
      for (j = 0; j < nstream; j++)
         engine->condition.parameter_iw[i][j] = average_weight;
   }
   engine->condition.gv_iw = (double **) HTS_calloc(num_voices, sizeof(double *));
   for (i = 0; i < num_voices; i++) {
      engine->condition.gv_iw[i] = (double *) HTS_calloc(nstream, sizeof(double));
      for (j = 0; j < nstream; j++)
         engine->condition.gv_iw[i][j] = average_weight;
   }

   return TRUE;
}

/* HTS_Engine_set_sampling_frequency: set sampling frequency */
void HTS_Engine_set_sampling_frequency(HTS_Engine * engine, size_t i)
{
   if (i < 1)
      i = 1;
   engine->condition.sampling_frequency = i;
   HTS_Audio_set_parameter(&engine->audio, engine->condition.sampling_frequency, engine->condition.audio_buff_size);
}

/* HTS_Engine_get_sampling_frequency: get sampling frequency */
size_t HTS_Engine_get_sampling_frequency(HTS_Engine * engine)
{
   return engine->condition.sampling_frequency;
}

/* HTS_Engine_set_fperiod: set frame period */
void HTS_Engine_set_fperiod(HTS_Engine * engine, size_t i)
{
   if (i < 1)
      i = 1;
   engine->condition.fperiod = i;
}

/* HTS_Engine_get_fperiod: get frame period */
size_t HTS_Engine_get_fperiod(HTS_Engine * engine)
{
   return engine->condition.fperiod;
}

/* HTS_Engine_set_audio_buff_size: set audio buffer size */
void HTS_Engine_set_audio_buff_size(HTS_Engine * engine, size_t i)
{
   engine->condition.audio_buff_size = i;
   HTS_Audio_set_parameter(&engine->audio, engine->condition.sampling_frequency, engine->condition.audio_buff_size);
}

/* HTS_Engine_get_audio_buff_size: get audio buffer size */
size_t HTS_Engine_get_audio_buff_size(HTS_Engine * engine)
{
   return engine->condition.audio_buff_size;
}

/* HTS_Engine_set_stop_flag: set stop flag */
void HTS_Engine_set_stop_flag(HTS_Engine * engine, HTS_Boolean b)
{
   engine->condition.stop = b;
}

/* HTS_Engine_get_stop_flag: get stop flag */
HTS_Boolean HTS_Engine_get_stop_flag(HTS_Engine * engine)
{
   return engine->condition.stop;
}

/* HTS_Engine_set_volume: set volume in db */
void HTS_Engine_set_volume(HTS_Engine * engine, double f)
{
   engine->condition.volume = exp(f * DB);
}

/* HTS_Engine_get_volume: get volume in db */
double HTS_Engine_get_volume(HTS_Engine * engine)
{
   return log(engine->condition.volume) / DB;
}

/* HTS_Egnine_set_msd_threshold: set MSD threshold */
void HTS_Engine_set_msd_threshold(HTS_Engine * engine, size_t stream_index, double f)
{
   if (f < 0.0)
      f = 0.0;
   if (f > 1.0)
      f = 1.0;
   engine->condition.msd_threshold[stream_index] = f;
}

/* HTS_Engine_get_msd_threshold: get MSD threshold */
double HTS_Engine_get_msd_threshold(HTS_Engine * engine, size_t stream_index)
{
   return engine->condition.msd_threshold[stream_index];
}

/* HTS_Engine_set_gv_weight: set GV weight */
void HTS_Engine_set_gv_weight(HTS_Engine * engine, size_t stream_index, double f)
{
   if (f < 0.0)
      f = 0.0;
   engine->condition.gv_weight[stream_index] = f;
}

/* HTS_Engine_get_gv_weight: get GV weight */
double HTS_Engine_get_gv_weight(HTS_Engine * engine, size_t stream_index)
{
   return engine->condition.gv_weight[stream_index];
}

/* HTS_Engine_set_speed: set speech speed */
void HTS_Engine_set_speed(HTS_Engine * engine, double f)
{
   if (f < 1.0E-06)
      f = 1.0E-06;
   engine->condition.speed = f;
}

/* HTS_Engine_set_phoneme_alignment_flag: set flag for using phoneme alignment in label */
void HTS_Engine_set_phoneme_alignment_flag(HTS_Engine * engine, HTS_Boolean b)
{
   engine->condition.phoneme_alignment_flag = b;
}

/* HTS_Engine_set_alpha: set alpha */
void HTS_Engine_set_alpha(HTS_Engine * engine, double f)
{
   if (f < 0.0)
      f = 0.0;
   if (f > 1.0)
      f = 1.0;
   engine->condition.alpha = f;
}

/* HTS_Engine_get_alpha: get alpha */
double HTS_Engine_get_alpha(HTS_Engine * engine)
{
   return engine->condition.alpha;
}

/* HTS_Engine_set_beta: set beta */
void HTS_Engine_set_beta(HTS_Engine * engine, double f)
{
   if (f < 0.0)
      f = 0.0;
   if (f > 1.0)
      f = 1.0;
   engine->condition.beta = f;
}

/* HTS_Engine_get_beta: get beta */
double HTS_Engine_get_beta(HTS_Engine * engine)
{
   return engine->condition.beta;
}

/* HTS_Engine_add_half_tone: add half tone */
void HTS_Engine_add_half_tone(HTS_Engine * engine, double f)
{
   engine->condition.additional_half_tone = f;
}

/* HTS_Engine_set_duration_interpolation_weight: set interpolation weight for duration */
void HTS_Engine_set_duration_interpolation_weight(HTS_Engine * engine, size_t voice_index, double f)
{
   engine->condition.duration_iw[voice_index] = f;
}

/* HTS_Engine_get_duration_interpolation_weight: get interpolation weight for duration */
double HTS_Engine_get_duration_interpolation_weight(HTS_Engine * engine, size_t voice_index)
{
   return engine->condition.duration_iw[voice_index];
}

/* HTS_Engine_set_parameter_interpolation_weight: set interpolation weight for parameter */
void HTS_Engine_set_parameter_interpolation_weight(HTS_Engine * engine, size_t voice_index, size_t stream_index, double f)
{
   engine->condition.parameter_iw[voice_index][stream_index] = f;
}

/* HTS_Engine_get_parameter_interpolation_weight: get interpolation weight for parameter */
double HTS_Engine_get_parameter_interpolation_weight(HTS_Engine * engine, size_t voice_index, size_t stream_index)
{
   return engine->condition.parameter_iw[voice_index][stream_index];
}

/* HTS_Engine_set_gv_interpolation_weight: set interpolation weight for GV */
void HTS_Engine_set_gv_interpolation_weight(HTS_Engine * engine, size_t voice_index, size_t stream_index, double f)
{
   engine->condition.gv_iw[voice_index][stream_index] = f;
}

/* HTS_Engine_get_gv_interpolation_weight: get interpolation weight for GV */
double HTS_Engine_get_gv_interpolation_weight(HTS_Engine * engine, size_t voice_index, size_t stream_index)
{
   return engine->condition.gv_iw[voice_index][stream_index];
}

/* HTS_Engine_get_total_state: get total number of state */
size_t HTS_Engine_get_total_state(HTS_Engine * engine)
{
   return HTS_SStreamSet_get_total_state(&engine->sss);
}

/* HTS_Engine_set_state_mean: set mean value of state */
void HTS_Engine_set_state_mean(HTS_Engine * engine, size_t stream_index, size_t state_index, size_t vector_index, double f)
{
   HTS_SStreamSet_set_mean(&engine->sss, stream_index, state_index, vector_index, f);
}

/* HTS_Engine_get_state_mean: get mean value of state */
double HTS_Engine_get_state_mean(HTS_Engine * engine, size_t stream_index, size_t state_index, size_t vector_index)
{
   return HTS_SStreamSet_get_mean(&engine->sss, stream_index, state_index, vector_index);
}

/* HTS_Engine_get_state_duration: get state duration */
size_t HTS_Engine_get_state_duration(HTS_Engine * engine, size_t state_index)
{
   return HTS_SStreamSet_get_duration(&engine->sss, state_index);
}

/* HTS_Engine_get_nvoices: get number of voices */
size_t HTS_Engine_get_nvoices(HTS_Engine * engine)
{
   return HTS_ModelSet_get_nvoices(&engine->ms);
}

/* HTS_Engine_get_nstream: get number of stream */
size_t HTS_Engine_get_nstream(HTS_Engine * engine)
{
   return HTS_ModelSet_get_nstream(&engine->ms);
}

/* HTS_Engine_get_nstate: get number of state */
size_t HTS_Engine_get_nstate(HTS_Engine * engine)
{
   return HTS_ModelSet_get_nstate(&engine->ms);
}

/* HTS_Engine_get_fullcontext_label_format: get full context label format */
const char *HTS_Engine_get_fullcontext_label_format(HTS_Engine * engine)
{
   return HTS_ModelSet_get_fullcontext_label_format(&engine->ms);
}

/* HTS_Engine_get_fullcontext_label_version: get full context label version */
const char *HTS_Engine_get_fullcontext_label_version(HTS_Engine * engine)
{
   return HTS_ModelSet_get_fullcontext_label_version(&engine->ms);
}

/* HTS_Engine_get_total_frame: get total number of frame */
size_t HTS_Engine_get_total_frame(HTS_Engine * engine)
{
   return HTS_GStreamSet_get_total_frame(&engine->gss);
}

/* HTS_Engine_get_nsamples: get number of samples */
size_t HTS_Engine_get_nsamples(HTS_Engine * engine)
{
   return HTS_GStreamSet_get_total_nsamples(&engine->gss);
}

/* HTS_Engine_get_generated_parameter: output generated parameter */
double HTS_Engine_get_generated_parameter(HTS_Engine * engine, size_t stream_index, size_t frame_index, size_t vector_index)
{
   return HTS_GStreamSet_get_parameter(&engine->gss, stream_index, frame_index, vector_index);
}

/* HTS_Engine_get_generated_speech: output generated speech */
double HTS_Engine_get_generated_speech(HTS_Engine * engine, size_t index)
{
   return HTS_GStreamSet_get_speech(&engine->gss, index);
}

/* HTS_Engine_generate_state_sequence: genereate state sequence (1st synthesis step) */
static HTS_Boolean HTS_Engine_generate_state_sequence(HTS_Engine * engine)
{
   size_t i, state_index, model_index;
   double f;

   if (HTS_SStreamSet_create(&engine->sss, &engine->ms, &engine->label, engine->condition.phoneme_alignment_flag, engine->condition.speed, engine->condition.duration_iw, engine->condition.parameter_iw, engine->condition.gv_iw) != TRUE) {
      HTS_Engine_refresh(engine);
      return FALSE;
   }
   if (engine->condition.additional_half_tone != 0.0) {
      state_index = 0;
      model_index = 0;
      for (i = 0; i < HTS_Engine_get_total_state(engine); i++) {
         f = HTS_Engine_get_state_mean(engine, 1, i, 0);
         f += engine->condition.additional_half_tone * HALF_TONE;
         if (f < MIN_LF0)
            f = MIN_LF0;
         else if (f > MAX_LF0)
            f = MAX_LF0;
         HTS_Engine_set_state_mean(engine, 1, i, 0, f);
         state_index++;
         if (state_index >= HTS_Engine_get_nstate(engine)) {
            state_index = 0;
            model_index++;
         }
      }
   }
   return TRUE;
}

/* HTS_Engine_generate_state_sequence_from_fn: genereate state sequence from file name (1st synthesis step) */
HTS_Boolean HTS_Engine_generate_state_sequence_from_fn(HTS_Engine * engine, const char *fn)
{
   HTS_Engine_refresh(engine);
   HTS_Label_load_from_fn(&engine->label, engine->condition.sampling_frequency, engine->condition.fperiod, fn);
   return HTS_Engine_generate_state_sequence(engine);
}

/* HTS_Engine_generate_state_sequence_from_strings: generate state sequence from strings (1st synthesis step) */
HTS_Boolean HTS_Engine_generate_state_sequence_from_strings(HTS_Engine * engine, char **lines, size_t num_lines)
{
   HTS_Engine_refresh(engine);
   HTS_Label_load_from_strings(&engine->label, engine->condition.sampling_frequency, engine->condition.fperiod, lines, num_lines);
   return HTS_Engine_generate_state_sequence(engine);
}

/* HTS_Engine_generate_parameter_sequence: generate parameter sequence (2nd synthesis step) */
HTS_Boolean HTS_Engine_generate_parameter_sequence(HTS_Engine * engine)
{
   return HTS_PStreamSet_create(&engine->pss, &engine->sss, engine->condition.msd_threshold, engine->condition.gv_weight);
}

/* HTS_Engine_generate_sample_sequence: generate sample sequence (3rd synthesis step) */
HTS_Boolean HTS_Engine_generate_sample_sequence(HTS_Engine * engine)
{
   return HTS_GStreamSet_create(&engine->gss, &engine->pss, engine->condition.stage, engine->condition.use_log_gain, engine->condition.sampling_frequency, engine->condition.fperiod, engine->condition.alpha, engine->condition.beta, &engine->condition.stop, engine->condition.volume, engine->condition.audio_buff_size > 0 ? &engine->audio : NULL);
}

/* HTS_Engine_synthesize: synthesize speech */
static HTS_Boolean HTS_Engine_synthesize(HTS_Engine * engine)
{
   if (HTS_Engine_generate_state_sequence(engine) != TRUE) {
      HTS_Engine_refresh(engine);
      return FALSE;
   }
   if (HTS_Engine_generate_parameter_sequence(engine) != TRUE) {
      HTS_Engine_refresh(engine);
      return FALSE;
   }
   if (HTS_Engine_generate_sample_sequence(engine) != TRUE) {
      HTS_Engine_refresh(engine);
      return FALSE;
   }
   return TRUE;
}

/* HTS_Engine_synthesize_from_fn: synthesize speech from file name */
HTS_Boolean HTS_Engine_synthesize_from_fn(HTS_Engine * engine, const char *fn)
{
   HTS_Engine_refresh(engine);
   HTS_Label_load_from_fn(&engine->label, engine->condition.sampling_frequency, engine->condition.fperiod, fn);
   return HTS_Engine_synthesize(engine);
}

/* HTS_Engine_synthesize_from_strings: synthesize speech from strings */
HTS_Boolean HTS_Engine_synthesize_from_strings(HTS_Engine * engine, char **lines, size_t num_lines)
{
   HTS_Engine_refresh(engine);
   HTS_Label_load_from_strings(&engine->label, engine->condition.sampling_frequency, engine->condition.fperiod, lines, num_lines);
   return HTS_Engine_synthesize(engine);
}

/* HTS_Engine_save_information: save trace information */
void HTS_Engine_save_information(HTS_Engine * engine, FILE * fp)
{
   size_t i, j, k, l, m, n;
   double temp;
   HTS_Condition *condition = &engine->condition;
   HTS_ModelSet *ms = &engine->ms;
   HTS_Label *label = &engine->label;
   HTS_SStreamSet *sss = &engine->sss;
   HTS_PStreamSet *pss = &engine->pss;

   /* global parameter */
   fprintf(fp, "[Global parameter]\n");
   fprintf(fp, "Sampring frequency                     -> %8lu(Hz)\n", (unsigned long) condition->sampling_frequency);
   fprintf(fp, "Frame period                           -> %8lu(point)\n", (unsigned long) condition->fperiod);
   fprintf(fp, "                                          %8.5f(msec)\n", 1e+3 * condition->fperiod / condition->sampling_frequency);
   fprintf(fp, "All-pass constant                      -> %8.5f\n", (float) condition->alpha);
   fprintf(fp, "Gamma                                  -> %8.5f\n", (float) (condition->stage == 0 ? 0.0 : -1.0 / condition->stage));
   if (condition->stage != 0) {
      if (condition->use_log_gain == TRUE)
         fprintf(fp, "Log gain flag                          ->     TRUE\n");
      else
         fprintf(fp, "Log gain flag                          ->    FALSE\n");
   }
   fprintf(fp, "Postfiltering coefficient              -> %8.5f\n", (float) condition->beta);
   fprintf(fp, "Audio buffer size                      -> %8lu(sample)\n", (unsigned long) condition->audio_buff_size);
   fprintf(fp, "\n");

   /* duration parameter */
   fprintf(fp, "[Duration parameter]\n");
   fprintf(fp, "Number of states                       -> %8lu\n", (unsigned long) HTS_ModelSet_get_nstate(ms));
   fprintf(fp, "         Interpolation size            -> %8lu\n", (unsigned long) HTS_ModelSet_get_nvoices(ms));
   /* check interpolation */
   for (i = 0, temp = 0.0; i < HTS_ModelSet_get_nvoices(ms); i++)
      temp += condition->duration_iw[i];
   for (i = 0; i < HTS_ModelSet_get_nvoices(ms); i++)
      if (condition->duration_iw[i] != 0.0)
         condition->duration_iw[i] /= temp;
   for (i = 0; i < HTS_ModelSet_get_nvoices(ms); i++)
      fprintf(fp, "         Interpolation weight[%2lu]      -> %8.0f(%%)\n", (unsigned long) i, (float) (100 * condition->duration_iw[i]));
   fprintf(fp, "\n");

   fprintf(fp, "[Stream parameter]\n");
   for (i = 0; i < HTS_ModelSet_get_nstream(ms); i++) {
      /* stream parameter */
      fprintf(fp, "Stream[%2lu] vector length               -> %8lu\n", (unsigned long) i, (unsigned long) HTS_ModelSet_get_vector_length(ms, i));
      fprintf(fp, "           Dynamic window size         -> %8lu\n", (unsigned long) HTS_ModelSet_get_window_size(ms, i));
      /* interpolation */
      fprintf(fp, "           Interpolation size          -> %8lu\n", (unsigned long) HTS_ModelSet_get_nvoices(ms));
      for (j = 0, temp = 0.0; j < HTS_ModelSet_get_nvoices(ms); j++)
         temp += condition->parameter_iw[j][i];
      for (j = 0; j < HTS_ModelSet_get_nvoices(ms); j++)
         if (condition->parameter_iw[j][i] != 0.0)
            condition->parameter_iw[j][i] /= temp;
      for (j = 0; j < HTS_ModelSet_get_nvoices(ms); j++)
         fprintf(fp, "           Interpolation weight[%2lu]    -> %8.0f(%%)\n", (unsigned long) j, (float) (100 * condition->parameter_iw[j][i]));
      /* MSD */
      if (HTS_ModelSet_is_msd(ms, i)) { /* for MSD */
         fprintf(fp, "           MSD flag                    ->     TRUE\n");
         fprintf(fp, "           MSD threshold               -> %8.5f\n", condition->msd_threshold[i]);
      } else {                  /* for non MSD */
         fprintf(fp, "           MSD flag                    ->    FALSE\n");
      }
      /* GV */
      if (HTS_ModelSet_use_gv(ms, i)) {
         fprintf(fp, "           GV flag                     ->     TRUE\n");
         fprintf(fp, "           GV weight                   -> %8.0f(%%)\n", (float) (100 * condition->gv_weight[i]));
         fprintf(fp, "           GV interpolation size       -> %8lu\n", (unsigned long) HTS_ModelSet_get_nvoices(ms));
         /* interpolation */
         for (j = 0, temp = 0.0; j < HTS_ModelSet_get_nvoices(ms); j++)
            temp += condition->gv_iw[j][i];
         for (j = 0; j < HTS_ModelSet_get_nvoices(ms); j++)
            if (condition->gv_iw[j][i] != 0.0)
               condition->gv_iw[j][i] /= temp;
         for (j = 0; j < HTS_ModelSet_get_nvoices(ms); j++)
            fprintf(fp, "           GV interpolation weight[%2lu] -> %8.0f(%%)\n", (unsigned long) j, (float) (100 * condition->gv_iw[j][i]));
      } else {
         fprintf(fp, "           GV flag                     ->    FALSE\n");
      }
   }
   fprintf(fp, "\n");

   /* generated sequence */
   fprintf(fp, "[Generated sequence]\n");
   fprintf(fp, "Number of HMMs                         -> %8lu\n", (unsigned long) HTS_Label_get_size(label));
   fprintf(fp, "Number of stats                        -> %8lu\n", (unsigned long) HTS_Label_get_size(label) * HTS_ModelSet_get_nstate(ms));
   fprintf(fp, "Length of this speech                  -> %8.3f(sec)\n", (float) ((double) HTS_PStreamSet_get_total_frame(pss) * condition->fperiod / condition->sampling_frequency));
   fprintf(fp, "                                       -> %8lu(frames)\n", (unsigned long) HTS_PStreamSet_get_total_frame(pss) * condition->fperiod);

   for (i = 0; i < HTS_Label_get_size(label); i++) {
      fprintf(fp, "HMM[%2lu]\n", (unsigned long) i);
      fprintf(fp, "  Name                                 -> %s\n", HTS_Label_get_string(label, i));
      fprintf(fp, "  Duration\n");
      for (j = 0; j < HTS_ModelSet_get_nvoices(ms); j++) {
         fprintf(fp, "    Interpolation[%2lu]\n", (unsigned long) j);
         HTS_ModelSet_get_duration_index(ms, j, HTS_Label_get_string(label, i), &k, &l);
         fprintf(fp, "      Tree index                       -> %8lu\n", (unsigned long) k);
         fprintf(fp, "      PDF index                        -> %8lu\n", (unsigned long) l);
      }
      for (j = 0; j < HTS_ModelSet_get_nstate(ms); j++) {
         fprintf(fp, "  State[%2lu]\n", (unsigned long) j + 2);
         fprintf(fp, "    Length                             -> %8lu(frames)\n", (unsigned long) HTS_SStreamSet_get_duration(sss, i * HTS_ModelSet_get_nstate(ms) + j));
         for (k = 0; k < HTS_ModelSet_get_nstream(ms); k++) {
            fprintf(fp, "    Stream[%2lu]\n", (unsigned long) k);
            if (HTS_ModelSet_is_msd(ms, k)) {
               if (HTS_SStreamSet_get_msd(sss, k, i * HTS_ModelSet_get_nstate(ms) + j) > condition->msd_threshold[k])
                  fprintf(fp, "      MSD flag                         ->     TRUE\n");
               else
                  fprintf(fp, "      MSD flag                         ->    FALSE\n");
            }
            for (l = 0; l < HTS_ModelSet_get_nvoices(ms); l++) {
               fprintf(fp, "      Interpolation[%2lu]\n", (unsigned long) l);
               HTS_ModelSet_get_parameter_index(ms, l, k, j + 2, HTS_Label_get_string(label, i), &m, &n);
               fprintf(fp, "        Tree index                     -> %8lu\n", (unsigned long) m);
               fprintf(fp, "        PDF index                      -> %8lu\n", (unsigned long) n);
            }
         }
      }
   }
}

/* HTS_Engine_save_label: save label with time */
void HTS_Engine_save_label(HTS_Engine * engine, FILE * fp)
{
   size_t i, j;
   size_t frame, state, duration;

   HTS_Label *label = &engine->label;
   HTS_SStreamSet *sss = &engine->sss;
   size_t nstate = HTS_ModelSet_get_nstate(&engine->ms);
   double rate = engine->condition.fperiod * 1.0e+07 / engine->condition.sampling_frequency;

   for (i = 0, state = 0, frame = 0; i < HTS_Label_get_size(label); i++) {
      for (j = 0, duration = 0; j < nstate; j++)
         duration += HTS_SStreamSet_get_duration(sss, state++);
      fprintf(fp, "%lu %lu %s\n", (unsigned long) (frame * rate), (unsigned long) ((frame + duration) * rate), HTS_Label_get_string(label, i));
      frame += duration;
   }
}

/* HTS_Engine_save_generated_parameter: save generated parameter */
void HTS_Engine_save_generated_parameter(HTS_Engine * engine, size_t stream_index, FILE * fp)
{
   size_t i, j;
   float temp;
   HTS_GStreamSet *gss = &engine->gss;

   for (i = 0; i < HTS_GStreamSet_get_total_frame(gss); i++)
      for (j = 0; j < HTS_GStreamSet_get_vector_length(gss, stream_index); j++) {
         temp = (float) HTS_GStreamSet_get_parameter(gss, stream_index, i, j);
         fwrite(&temp, sizeof(float), 1, fp);
      }
}

/* HTS_Engine_save_generated_speech: save generated speech */
void HTS_Engine_save_generated_speech(HTS_Engine * engine, FILE * fp)
{
   size_t i;
   double x;
   short temp;
   HTS_GStreamSet *gss = &engine->gss;

   for (i = 0; i < HTS_GStreamSet_get_total_nsamples(gss); i++) {
      x = HTS_GStreamSet_get_speech(gss, i);
      if (x > 32767.0)
         temp = 32767;
      else if (x < -32768.0)
         temp = -32768;
      else
         temp = (short) x;
      fwrite(&temp, sizeof(short), 1, fp);
   }
}

/* HTS_Engine_save_riff: save RIFF format file */
void HTS_Engine_save_riff(HTS_Engine * engine, FILE * fp)
{
   size_t i;
   double x;
   short temp;

   HTS_GStreamSet *gss = &engine->gss;
   char data_01_04[] = { 'R', 'I', 'F', 'F' };
   int data_05_08 = HTS_GStreamSet_get_total_nsamples(gss) * sizeof(short) + 36;
   char data_09_12[] = { 'W', 'A', 'V', 'E' };
   char data_13_16[] = { 'f', 'm', 't', ' ' };
   int data_17_20 = 16;
   short data_21_22 = 1;        /* PCM */
   short data_23_24 = 1;        /* monoral */
   int data_25_28 = engine->condition.sampling_frequency;
   int data_29_32 = engine->condition.sampling_frequency * sizeof(short);
   short data_33_34 = sizeof(short);
   short data_35_36 = (short) (sizeof(short) * 8);
   char data_37_40[] = { 'd', 'a', 't', 'a' };
   int data_41_44 = HTS_GStreamSet_get_total_nsamples(gss) * sizeof(short);

   /* write header */
   HTS_fwrite_little_endian(data_01_04, sizeof(char), 4, fp);
   HTS_fwrite_little_endian(&data_05_08, sizeof(int), 1, fp);
   HTS_fwrite_little_endian(data_09_12, sizeof(char), 4, fp);
   HTS_fwrite_little_endian(data_13_16, sizeof(char), 4, fp);
   HTS_fwrite_little_endian(&data_17_20, sizeof(int), 1, fp);
   HTS_fwrite_little_endian(&data_21_22, sizeof(short), 1, fp);
   HTS_fwrite_little_endian(&data_23_24, sizeof(short), 1, fp);
   HTS_fwrite_little_endian(&data_25_28, sizeof(int), 1, fp);
   HTS_fwrite_little_endian(&data_29_32, sizeof(int), 1, fp);
   HTS_fwrite_little_endian(&data_33_34, sizeof(short), 1, fp);
   HTS_fwrite_little_endian(&data_35_36, sizeof(short), 1, fp);
   HTS_fwrite_little_endian(data_37_40, sizeof(char), 4, fp);
   HTS_fwrite_little_endian(&data_41_44, sizeof(int), 1, fp);
   /* write data */
   for (i = 0; i < HTS_GStreamSet_get_total_nsamples(gss); i++) {
      x = HTS_GStreamSet_get_speech(gss, i);
      if (x > 32767.0)
         temp = 32767;
      else if (x < -32768.0)
         temp = -32768;
      else
         temp = (short) x;
      HTS_fwrite_little_endian(&temp, sizeof(short), 1, fp);
   }
}

/* HTS_Engine_refresh: free model per one time synthesis */
void HTS_Engine_refresh(HTS_Engine * engine)
{
   /* free generated parameter stream set */
   HTS_GStreamSet_clear(&engine->gss);
   /* free parameter stream set */
   HTS_PStreamSet_clear(&engine->pss);
   /* free state stream set */
   HTS_SStreamSet_clear(&engine->sss);
   /* free label list */
   HTS_Label_clear(&engine->label);
   /* stop flag */
   engine->condition.stop = FALSE;
}

/* HTS_Engine_clear: free engine */
void HTS_Engine_clear(HTS_Engine * engine)
{
   size_t i;

   if (engine->condition.msd_threshold != NULL)
      HTS_free(engine->condition.msd_threshold);
   if (engine->condition.duration_iw != NULL)
      HTS_free(engine->condition.duration_iw);
   if (engine->condition.gv_weight != NULL)
      HTS_free(engine->condition.gv_weight);
   if (engine->condition.parameter_iw != NULL) {
      for (i = 0; i < HTS_ModelSet_get_nvoices(&engine->ms); i++)
         HTS_free(engine->condition.parameter_iw[i]);
      HTS_free(engine->condition.parameter_iw);
   }
   if (engine->condition.gv_iw != NULL) {
      for (i = 0; i < HTS_ModelSet_get_nvoices(&engine->ms); i++)
         HTS_free(engine->condition.gv_iw[i]);
      HTS_free(engine->condition.gv_iw);
   }

   HTS_ModelSet_clear(&engine->ms);
   HTS_Audio_clear(&engine->audio);
   HTS_Engine_initialize(engine);
}

HTS_ENGINE_C_END;

#endif                          /* !HTS_ENGINE_C */
