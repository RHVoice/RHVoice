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

#ifndef HTS_LABEL_C
#define HTS_LABEL_C

#ifdef __cplusplus
#define HTS_LABEL_C_START extern "C" {
#define HTS_LABEL_C_END   }
#else
#define HTS_LABEL_C_START
#define HTS_LABEL_C_END
#endif                          /* __CPLUSPLUS */

HTS_LABEL_C_START;

#include <stdlib.h>             /* for atof() */
#include <ctype.h>              /* for isgraph(),isdigit() */

/* hts_engine libraries */
#include "HTS_hidden.h"

static HTS_Boolean isdigit_string(char *str)
{
   int i;

   if (sscanf(str, "%d", &i) == 1)
      return TRUE;
   else
      return FALSE;
}

/* HTS_Label_initialize: initialize label */
void HTS_Label_initialize(HTS_Label * label)
{
   label->head = NULL;
   label->size = 0;
   label->frame_flag = FALSE;
   label->speech_speed = 1.0;
}

/* HTS_Label_check_time: check label */
static void HTS_Label_check_time(HTS_Label * label)
{
   HTS_LabelString *lstring = label->head;
   HTS_LabelString *next = NULL;

   if (lstring)
      lstring->start = 0.0;
   while (lstring) {
      next = lstring->next;
      if (!next)
         break;
      if (lstring->end < 0.0 && next->start >= 0.0)
         lstring->end = next->start;
      else if (lstring->end >= 0.0 && next->start < 0.0)
         next->start = lstring->end;
      if (lstring->start < 0.0)
         lstring->start = -1.0;
      if (lstring->end < 0.0)
         lstring->end = -1.0;
      lstring = next;
   }
}

/* HTS_Label_load_from_fn: load label from file name */
void HTS_Label_load_from_fn(HTS_Label * label, int sampling_rate, int fperiod, char *fn)
{
   HTS_File *fp = HTS_fopen(fn, "r");
   HTS_Label_load_from_fp(label, sampling_rate, fperiod, fp);
   HTS_fclose(fp);
}

/* HTS_Label_load_from_fp: load label from file pointer */
void HTS_Label_load_from_fp(HTS_Label * label, int sampling_rate, int fperiod, HTS_File * fp)
{
   char buff[HTS_MAXBUFLEN];
   HTS_LabelString *lstring = NULL;
   double start, end;
   const double rate = (double) sampling_rate / ((double) fperiod * 1e+7);

   if (label->head || label->size != 0) {
      HTS_error(1, "HTS_Label_load_from_fp: label is not initialized.\n");
      return;
   }
   /* parse label file */
   while (HTS_get_token(fp, buff)) {
      if (!isgraph((int) buff[0]))
         break;
      label->size++;

      if (lstring) {
         lstring->next = (HTS_LabelString *) HTS_calloc(1, sizeof(HTS_LabelString));
         lstring = lstring->next;
      } else {                  /* first time */
         lstring = (HTS_LabelString *) HTS_calloc(1, sizeof(HTS_LabelString));
         label->head = lstring;
      }
      if (isdigit_string(buff)) {       /* has frame infomation */
         start = atof(buff);
         HTS_get_token(fp, buff);
         end = atof(buff);
         HTS_get_token(fp, buff);
         lstring->start = rate * start;
         lstring->end = rate * end;
      } else {
         lstring->start = -1.0;
         lstring->end = -1.0;
      }
      lstring->next = NULL;
      lstring->name = HTS_strdup(buff);
   }
   HTS_Label_check_time(label);
}

/* HTS_Label_load_from_string: load label from string */
void HTS_Label_load_from_string(HTS_Label * label, int sampling_rate, int fperiod, char *data)
{
   char buff[HTS_MAXBUFLEN];
   HTS_LabelString *lstring = NULL;
   int data_index = 0;          /* data index */
   double start, end;
   const double rate = (double) sampling_rate / ((double) fperiod * 1e+7);

   if (label->head || label->size != 0) {
      HTS_error(1, "HTS_Label_load_from_fp: label list is not initialized.\n");
      return;
   }
   /* copy label */
   while (HTS_get_token_from_string(data, &data_index, buff)) {
      if (!isgraph((int) buff[0]))
         break;
      label->size++;

      if (lstring) {
         lstring->next = (HTS_LabelString *) HTS_calloc(1, sizeof(HTS_LabelString));
         lstring = lstring->next;
      } else {                  /* first time */
         lstring = (HTS_LabelString *) HTS_calloc(1, sizeof(HTS_LabelString));
         label->head = lstring;
      }
      if (isdigit_string(buff)) {       /* has frame infomation */
         start = atof(buff);
         HTS_get_token_from_string(data, &data_index, buff);
         end = atof(buff);
         HTS_get_token_from_string(data, &data_index, buff);
         lstring->start = rate * start;
         lstring->end = rate * end;
      } else {
         lstring->start = -1.0;
         lstring->end = -1.0;
      }
      lstring->next = NULL;
      lstring->name = HTS_strdup(buff);
   }
   HTS_Label_check_time(label);
}

/* HTS_Label_load_from_string_list: load label from string list */
void HTS_Label_load_from_string_list(HTS_Label * label, int sampling_rate, int fperiod, char **data, int size)
{
   char buff[HTS_MAXBUFLEN];
   HTS_LabelString *lstring = NULL;
   int i;
   int data_index;
   double start, end;
   const double rate = (double) sampling_rate / ((double) fperiod * 1e+7);

   if (label->head || label->size != 0) {
      HTS_error(1, "HTS_Label_load_from_fp: label list is not initialized.\n");
      return;
   }
   /* copy label */
   for (i = 0; i < size; i++) {
      if (!isgraph((int) data[i][0]))
         break;
      label->size++;

      if (lstring) {
         lstring->next = (HTS_LabelString *) HTS_calloc(1, sizeof(HTS_LabelString));
         lstring = lstring->next;
      } else {                  /* first time */
         lstring = (HTS_LabelString *) HTS_calloc(1, sizeof(HTS_LabelString));
         label->head = lstring;
      }
      data_index = 0;
      if (isdigit_string(data[i])) {    /* has frame infomation */
         HTS_get_token_from_string(data[i], &data_index, buff);
         start = atof(buff);
         HTS_get_token_from_string(data[i], &data_index, buff);
         end = atof(buff);
         HTS_get_token_from_string(data[i], &data_index, buff);
         lstring->name = HTS_strdup(buff);
         lstring->start = rate * start;
         lstring->end = rate * end;
      } else {
         lstring->start = -1.0;
         lstring->end = -1.0;
         lstring->name = HTS_strdup(data[i]);
      }
      lstring->next = NULL;
   }
   HTS_Label_check_time(label);
}

/* HTS_Label_set_frame_specified_flag: set frame specified flag */
void HTS_Label_set_frame_specified_flag(HTS_Label * label, HTS_Boolean i)
{
   label->frame_flag = i;
}

/* HTS_Label_set_speech_speed: set speech speed rate */
void HTS_Label_set_speech_speed(HTS_Label * label, double f)
{
   if (f > 0.0 && f <= 10.0)
      label->speech_speed = f;
}

/* HTS_Label_get_size: get number of label string */
int HTS_Label_get_size(HTS_Label * label)
{
   return label->size;
}

/* HTS_Label_get_string: get label string */
char *HTS_Label_get_string(HTS_Label * label, int string_index)
{
   HTS_LabelString *lstring = label->head;

   while (string_index-- && lstring)
      lstring = lstring->next;
   if (!lstring)
      return NULL;
   return lstring->name;
}

/* HTS_Label_get_frame_specified_flag: get frame specified flag */
HTS_Boolean HTS_Label_get_frame_specified_flag(HTS_Label * label)
{
   return label->frame_flag;
}

/* HTS_Label_get_start_frame: get start frame */
double HTS_Label_get_start_frame(HTS_Label * label, int string_index)
{
   HTS_LabelString *lstring = label->head;

   while (string_index-- && lstring)
      lstring = lstring->next;
   if (!lstring)
      return -1.0;
   return lstring->start;
}

/* HTS_Label_get_end_frame: get end frame */
double HTS_Label_get_end_frame(HTS_Label * label, int string_index)
{
   HTS_LabelString *lstring = label->head;

   while (string_index-- && lstring)
      lstring = lstring->next;
   if (!lstring)
      return -1.0;
   return lstring->end;
}

/* HTS_Label_get_speech_speed: get speech speed rate */
double HTS_Label_get_speech_speed(HTS_Label * label)
{
   return label->speech_speed;
}

/* HTS_Label_clear: free label */
void HTS_Label_clear(HTS_Label * label)
{
   HTS_LabelString *lstring, *next_lstring;

   for (lstring = label->head; lstring; lstring = next_lstring) {
      next_lstring = lstring->next;
      HTS_free(lstring->name);
      HTS_free(lstring);
   }
   HTS_Label_initialize(label);
}

HTS_LABEL_C_END;

#endif                          /* !HTS_LABEL_C */
