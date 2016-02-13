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

#ifndef HTS_MODEL_C
#define HTS_MODEL_C

#ifdef __cplusplus
#define HTS_MODEL_C_START extern "C" {
#define HTS_MODEL_C_END   }
#else
#define HTS_MODEL_C_START
#define HTS_MODEL_C_END
#endif                          /* __CPLUSPLUS */

HTS_MODEL_C_START;

#include <stdlib.h>             /* for atoi(),abs() */
#include <string.h>             /* for strlen(),strstr(),strrchr(),strcmp() */
#include <ctype.h>              /* for isdigit() */

/* hts_engine libraries */
#include "HTS_hidden.h"

#ifdef WIN32
typedef unsigned __int32 uint32_t;
#else
#include <stdint.h>
#endif                          /* WIN32 */

/* HTS_dp_match: recursive matching */
static HTS_Boolean HTS_dp_match(const char *string, const char *pattern, size_t pos, size_t max)
{
   if (pos > max)
      return FALSE;
   if (string[0] == '\0' && pattern[0] == '\0')
      return TRUE;
   if (pattern[0] == '*') {
      if (HTS_dp_match(string + 1, pattern, pos + 1, max) == 1)
         return TRUE;
      else
         return HTS_dp_match(string, pattern + 1, pos, max);
   }
   if (string[0] == pattern[0] || pattern[0] == '?') {
      if (HTS_dp_match(string + 1, pattern + 1, pos + 1, max + 1) == 1)
         return TRUE;
   }

   return FALSE;
}

/* HTS_pattern_match: pattern matching function */
static HTS_Boolean HTS_pattern_match(const char *string, const char *pattern)
{
   size_t i, j;
   size_t buff_length, max = 0, nstar = 0, nquestion = 0;
   char buff[HTS_MAXBUFLEN];
   size_t pattern_length = strlen(pattern);

   for (i = 0; i < pattern_length; i++) {
      switch (pattern[i]) {
      case '*':
         nstar++;
         break;
      case '?':
         nquestion++;
         max++;
         break;
      default:
         max++;
      }
   }
   if (nstar == 2 && nquestion == 0 && pattern[0] == '*' && pattern[i - 1] == '*') {
      /* only string matching is required */
      buff_length = i - 2;
      for (i = 0, j = 1; i < buff_length; i++, j++)
         buff[i] = pattern[j];
      buff[buff_length] = '\0';
      if (strstr(string, buff) != NULL)
         return TRUE;
      else
         return FALSE;
   } else
      return HTS_dp_match(string, pattern, 0, strlen(string) - max);
}

/* HTS_is_num: check given buffer is number or not */
static HTS_Boolean HTS_is_num(const char *buff)
{
   size_t i;
   size_t length = strlen(buff);

   for (i = 0; i < length; i++)
      if (!(isdigit((int) buff[i]) || (buff[i] == '-')))
         return FALSE;

   return TRUE;
}

/* HTS_name2num: convert name of node to number */
static size_t HTS_name2num(const char *buff)
{
   size_t i;

   for (i = strlen(buff) - 1; '0' <= buff[i] && buff[i] <= '9'; i--);
   i++;

   return (size_t) atoi(&buff[i]);
}

/* HTS_get_state_num: return the number of state */
static size_t HTS_get_state_num(const char *string)
{
   const char *left, *right;

   left = strchr(string, '[');
   if (left == NULL)
      return 0;
   left++;

   right = strchr(left, ']');
   if (right == NULL)
      return 0;

   return (size_t) atoi(left);
}

/* HTS_Question_initialize: initialize question */
static void HTS_Question_initialize(HTS_Question * question)
{
   question->string = NULL;
   question->head = NULL;
   question->next = NULL;
}

/* HTS_Question_clear: clear loaded question */
static void HTS_Question_clear(HTS_Question * question)
{
   HTS_Pattern *pattern, *next_pattern;

   if (question->string != NULL)
      HTS_free(question->string);
   for (pattern = question->head; pattern; pattern = next_pattern) {
      next_pattern = pattern->next;
      HTS_free(pattern->string);
      HTS_free(pattern);
   }
   HTS_Question_initialize(question);
}

/* HTS_Question_load: Load questions from file */
static HTS_Boolean HTS_Question_load(HTS_Question * question, HTS_File * fp)
{
   char buff[HTS_MAXBUFLEN];
   HTS_Pattern *pattern, *last_pattern;

   if (question == NULL || fp == NULL)
      return FALSE;

   HTS_Question_clear(question);

   /* get question name */
   if (HTS_get_pattern_token(fp, buff) == FALSE)
      return FALSE;
   question->string = HTS_strdup(buff);

   /* get pattern list */
   if (HTS_get_pattern_token(fp, buff) == FALSE) {
      HTS_Question_clear(question);
      return FALSE;
   }

   last_pattern = NULL;
   if (strcmp(buff, "{") == 0) {
      while (1) {
         if (HTS_get_pattern_token(fp, buff) == FALSE) {
            HTS_Question_clear(question);
            return FALSE;
         }
         pattern = (HTS_Pattern *) HTS_calloc(1, sizeof(HTS_Pattern));
         if (question->head != NULL)
            last_pattern->next = pattern;
         else                   /* first time */
            question->head = pattern;
         pattern->string = HTS_strdup(buff);
         pattern->next = NULL;
         if (HTS_get_pattern_token(fp, buff) == FALSE) {
            HTS_Question_clear(question);
            return FALSE;
         }
         if (!strcmp(buff, "}"))
            break;
         last_pattern = pattern;
      }
   }
   return TRUE;
}

/* HTS_Question_match: check given string match given question */
static HTS_Boolean HTS_Question_match(HTS_Question * question, const char *string)
{
   HTS_Pattern *pattern;

   for (pattern = question->head; pattern; pattern = pattern->next)
      if (HTS_pattern_match(string, pattern->string))
         return TRUE;

   return FALSE;
}

/* HTS_Question_find: find question from question list */
static HTS_Question *HTS_Question_find(HTS_Question * question, const char *string)
{
   for (; question; question = question->next)
      if (strcmp(string, question->string) == 0)
         return question;

   return NULL;
}

/* HTS_Node_initialzie: initialize node */
static void HTS_Node_initialize(HTS_Node * node)
{
   node->index = 0;
   node->pdf = 0;
   node->yes = NULL;
   node->no = NULL;
   node->next = NULL;
   node->quest = NULL;
}

/* HTS_Node_clear: recursive function to free node */
static void HTS_Node_clear(HTS_Node * node)
{
   if (node->yes != NULL) {
      HTS_Node_clear(node->yes);
      HTS_free(node->yes);
   }
   if (node->no != NULL) {
      HTS_Node_clear(node->no);
      HTS_free(node->no);
   }
   HTS_Node_initialize(node);
}

/* HTS_Node_find: find node for given number */
static HTS_Node *HTS_Node_find(HTS_Node * node, int num)
{
   for (; node; node = node->next)
      if (node->index == num)
         return node;

   return NULL;
}

/* HTS_Tree_initialize: initialize tree */
static void HTS_Tree_initialize(HTS_Tree * tree)
{
   tree->head = NULL;
   tree->next = NULL;
   tree->root = NULL;
   tree->state = 0;
}

/* HTS_Tree_clear: clear given tree */
static void HTS_Tree_clear(HTS_Tree * tree)
{
   HTS_Pattern *pattern, *next_pattern;

   for (pattern = tree->head; pattern; pattern = next_pattern) {
      next_pattern = pattern->next;
      HTS_free(pattern->string);
      HTS_free(pattern);
   }
   if (tree->root != NULL) {
      HTS_Node_clear(tree->root);
      HTS_free(tree->root);
   }
   HTS_Tree_initialize(tree);
}

/* HTS_Tree_parse_pattern: parse pattern specified for each tree */
static void HTS_Tree_parse_pattern(HTS_Tree * tree, char *string)
{
   char *left, *right;
   HTS_Pattern *pattern, *last_pattern;

   tree->head = NULL;
   last_pattern = NULL;
   /* parse tree pattern */
   if ((left = strchr(string, '{')) != NULL) {  /* pattern is specified */
      string = left + 1;
      if (*string == '(')
         ++string;

      right = strrchr(string, '}');
      if (string < right && *(right - 1) == ')')
         --right;
      *right = ',';

      /* parse pattern */
      while ((left = strchr(string, ',')) != NULL) {
         pattern = (HTS_Pattern *) HTS_calloc(1, sizeof(HTS_Pattern));
         if (tree->head) {
            last_pattern->next = pattern;
         } else {
            tree->head = pattern;
         }
         *left = '\0';
         pattern->string = HTS_strdup(string);
         string = left + 1;
         pattern->next = NULL;
         last_pattern = pattern;
      }
   }
}

/* HTS_Tree_load: load trees */
static HTS_Boolean HTS_Tree_load(HTS_Tree * tree, HTS_File * fp, HTS_Question * question)
{
   char buff[HTS_MAXBUFLEN];
   HTS_Node *node, *last_node;

   if (tree == NULL || fp == NULL)
      return FALSE;

   if (HTS_get_pattern_token(fp, buff) == FALSE) {
      HTS_Tree_clear(tree);
      return FALSE;
   }
   node = (HTS_Node *) HTS_calloc(1, sizeof(HTS_Node));
   HTS_Node_initialize(node);
   tree->root = last_node = node;

   if (strcmp(buff, "{") == 0) {
      while (HTS_get_pattern_token(fp, buff) == TRUE && strcmp(buff, "}") != 0) {
         node = HTS_Node_find(last_node, atoi(buff));
         if (node == NULL) {
            HTS_error(0, "HTS_Tree_load: Cannot find node %d.\n", atoi(buff));
            HTS_Tree_clear(tree);
            return FALSE;
         }
         if (HTS_get_pattern_token(fp, buff) == FALSE) {
            HTS_Tree_clear(tree);
            return FALSE;
         }
         node->quest = HTS_Question_find(question, buff);
         if (node->quest == NULL) {
            HTS_error(0, "HTS_Tree_load: Cannot find question %s.\n", buff);
            HTS_Tree_clear(tree);
            return FALSE;
         }
         node->yes = (HTS_Node *) HTS_calloc(1, sizeof(HTS_Node));
         node->no = (HTS_Node *) HTS_calloc(1, sizeof(HTS_Node));
         HTS_Node_initialize(node->yes);
         HTS_Node_initialize(node->no);

         if (HTS_get_pattern_token(fp, buff) == FALSE) {
            node->quest = NULL;
            free(node->yes);
            free(node->no);
            HTS_Tree_clear(tree);
            return FALSE;
         }
         if (HTS_is_num(buff))
            node->no->index = atoi(buff);
         else
            node->no->pdf = HTS_name2num(buff);
         node->no->next = last_node;
         last_node = node->no;

         if (HTS_get_pattern_token(fp, buff) == FALSE) {
            node->quest = NULL;
            free(node->yes);
            free(node->no);
            HTS_Tree_clear(tree);
            return FALSE;
         }
         if (HTS_is_num(buff))
            node->yes->index = atoi(buff);
         else
            node->yes->pdf = HTS_name2num(buff);
         node->yes->next = last_node;
         last_node = node->yes;
      }
   } else {
      node->pdf = HTS_name2num(buff);
   }

   return TRUE;
}

/* HTS_Node_search: tree search */
static size_t HTS_Tree_search_node(HTS_Tree * tree, const char *string)
{
   HTS_Node *node = tree->root;

   while (node != NULL) {
      if (node->quest == NULL)
         return node->pdf;
      if (HTS_Question_match(node->quest, string)) {
         if (node->yes->pdf > 0)
            return node->yes->pdf;
         node = node->yes;
      } else {
         if (node->no->pdf > 0)
            return node->no->pdf;
         node = node->no;
      }
   }

   HTS_error(0, "HTS_Tree_search_node: Cannot find node.\n");
   return 1;
}

/* HTS_Window_initialize: initialize dynamic window */
static void HTS_Window_initialize(HTS_Window * win)
{
   win->size = 0;
   win->l_width = NULL;
   win->r_width = NULL;
   win->coefficient = NULL;
   win->max_width = 0;
}

/* HTS_Window_clear: free dynamic window */
static void HTS_Window_clear(HTS_Window * win)
{
   size_t i;

   if (win->coefficient != NULL) {
      for (i = 0; i < win->size; i++) {
         win->coefficient[i] += win->l_width[i];
         HTS_free(win->coefficient[i]);
      }
      HTS_free(win->coefficient);
   }
   if (win->l_width)
      HTS_free(win->l_width);
   if (win->r_width)
      HTS_free(win->r_width);

   HTS_Window_initialize(win);
}

/* HTS_Window_load: load dynamic windows */
static HTS_Boolean HTS_Window_load(HTS_Window * win, HTS_File ** fp, size_t size)
{
   size_t i, j;
   size_t fsize, length;
   char buff[HTS_MAXBUFLEN];
   HTS_Boolean result = TRUE;

   /* check */
   if (win == NULL || fp == NULL || size == 0)
      return FALSE;

   win->size = size;
   win->l_width = (int *) HTS_calloc(win->size, sizeof(int));
   win->r_width = (int *) HTS_calloc(win->size, sizeof(int));
   win->coefficient = (double **) HTS_calloc(win->size, sizeof(double *));
   /* set delta coefficents */
   for (i = 0; i < win->size; i++) {
      if (HTS_get_token_from_fp(fp[i], buff) == FALSE) {
         result = FALSE;
         fsize = 1;
      } else {
         fsize = atoi(buff);
         if (fsize == 0) {
            result = FALSE;
            fsize = 1;
         }
      }
      /* read coefficients */
      win->coefficient[i] = (double *) HTS_calloc(fsize, sizeof(double));
      for (j = 0; j < fsize; j++) {
         if (HTS_get_token_from_fp(fp[i], buff) == FALSE) {
            result = FALSE;
            win->coefficient[i][j] = 0.0;
         } else {
            win->coefficient[i][j] = (double) atof(buff);
         }
      }
      /* set pointer */
      length = fsize / 2;
      win->coefficient[i] += length;
      win->l_width[i] = -1 * (int) length;
      win->r_width[i] = (int) length;
      if (fsize % 2 == 0)
         win->r_width[i]--;
   }
   /* calcurate max_width to determine size of band matrix */
   win->max_width = 0;
   for (i = 0; i < win->size; i++) {
      if (win->max_width < (size_t) abs(win->l_width[i]))
         win->max_width = abs(win->l_width[i]);
      if (win->max_width < (size_t) abs(win->r_width[i]))
         win->max_width = abs(win->r_width[i]);
   }

   if (result == FALSE) {
      HTS_Window_clear(win);
      return FALSE;
   }
   return TRUE;
}

/* HTS_Model_initialize: initialize model */
static void HTS_Model_initialize(HTS_Model * model)
{
   model->vector_length = 0;
   model->num_windows = 0;
   model->is_msd = FALSE;
   model->ntree = 0;
   model->npdf = NULL;
   model->pdf = NULL;
   model->tree = NULL;
   model->question = NULL;
}

/* HTS_Model_clear: free pdfs and trees */
static void HTS_Model_clear(HTS_Model * model)
{
   size_t i, j;
   HTS_Question *question, *next_question;
   HTS_Tree *tree, *next_tree;

   for (question = model->question; question; question = next_question) {
      next_question = question->next;
      HTS_Question_clear(question);
      HTS_free(question);
   }
   for (tree = model->tree; tree; tree = next_tree) {
      next_tree = tree->next;
      HTS_Tree_clear(tree);
      HTS_free(tree);
   }
   if (model->pdf) {
      for (i = 2; i <= model->ntree + 1; i++) {
         for (j = 1; j <= model->npdf[i]; j++) {
            HTS_free(model->pdf[i][j]);
         }
         model->pdf[i]++;
         HTS_free(model->pdf[i]);
      }
      model->pdf += 2;
      HTS_free(model->pdf);
   }
   if (model->npdf) {
      model->npdf += 2;
      HTS_free(model->npdf);
   }
   HTS_Model_initialize(model);
}

/* HTS_Model_load_tree: load trees */
static HTS_Boolean HTS_Model_load_tree(HTS_Model * model, HTS_File * fp)
{
   char buff[HTS_MAXBUFLEN];
   HTS_Question *question, *last_question;
   HTS_Tree *tree, *last_tree;
   size_t state;

   /* check */
   if (model == NULL) {
      HTS_error(0, "HTS_Model_load_tree: File for trees is not specified.\n");
      return FALSE;
   }

   if (fp == NULL) {
      model->ntree = 1;
      return TRUE;
   }

   model->ntree = 0;
   last_question = NULL;
   last_tree = NULL;
   while (!HTS_feof(fp)) {
      HTS_get_pattern_token(fp, buff);
      /* parse questions */
      if (strcmp(buff, "QS") == 0) {
         question = (HTS_Question *) HTS_calloc(1, sizeof(HTS_Question));
         HTS_Question_initialize(question);
         if (HTS_Question_load(question, fp) == FALSE) {
            free(question);
            HTS_Model_clear(model);
            return FALSE;
         }
         if (model->question)
            last_question->next = question;
         else
            model->question = question;
         question->next = NULL;
         last_question = question;
      }
      /* parse trees */
      state = HTS_get_state_num(buff);
      if (state != 0) {
         tree = (HTS_Tree *) HTS_calloc(1, sizeof(HTS_Tree));
         HTS_Tree_initialize(tree);
         tree->state = state;
         HTS_Tree_parse_pattern(tree, buff);
         if (HTS_Tree_load(tree, fp, model->question) == FALSE) {
            free(tree);
            HTS_Model_clear(model);
            return FALSE;
         }
         if (model->tree)
            last_tree->next = tree;
         else
            model->tree = tree;
         tree->next = NULL;
         last_tree = tree;
         model->ntree++;
      }
   }
   /* No Tree information in tree file */
   if (model->tree == NULL)
      model->ntree = 1;

   return TRUE;
}

/* HTS_Model_load_pdf: load pdfs */
static HTS_Boolean HTS_Model_load_pdf(HTS_Model * model, HTS_File * fp, size_t vector_length, size_t num_windows, HTS_Boolean is_msd)
{
   uint32_t i;
   size_t j, k;
   HTS_Boolean result = TRUE;
   size_t len;

   /* check */
   if (model == NULL || fp == NULL || model->ntree <= 0) {
      HTS_error(1, "HTS_Model_load_pdf: File for pdfs is not specified.\n");
      return FALSE;
   }

   /* read MSD flag */
   model->vector_length = vector_length;
   model->num_windows = num_windows;
   model->is_msd = is_msd;
   model->npdf = (size_t *) HTS_calloc(model->ntree, sizeof(size_t));
   model->npdf -= 2;
   /* read the number of pdfs */
   for (j = 2; j <= model->ntree + 1; j++) {
      if (HTS_fread_little_endian(&i, sizeof(i), 1, fp) != 1) {
         result = FALSE;
         break;
      }
      model->npdf[j] = (size_t) i;
   }
   for (j = 2; j <= model->ntree + 1; j++) {
      if (model->npdf[j] <= 0) {
         HTS_error(1, "HTS_Model_load_pdf: # of pdfs at %d-th state should be positive.\n", j);
         result = FALSE;
         break;
      }
   }
   if (result == FALSE) {
      model->npdf += 2;
      free(model->npdf);
      HTS_Model_initialize(model);
      return FALSE;
   }
   model->pdf = (float ***) HTS_calloc(model->ntree, sizeof(float **));
   model->pdf -= 2;
   /* read means and variances */
   if (is_msd)                  /* for MSD */
      len = model->vector_length * model->num_windows * 2 + 1;
   else
      len = model->vector_length * model->num_windows * 2;
   for (j = 2; j <= model->ntree + 1; j++) {
      model->pdf[j] = (float **) HTS_calloc(model->npdf[j], sizeof(float *));
      model->pdf[j]--;
      for (k = 1; k <= model->npdf[j]; k++) {
         model->pdf[j][k] = (float *) HTS_calloc(len, sizeof(float));
         if (HTS_fread_little_endian(model->pdf[j][k], sizeof(float), len, fp) != len)
            result = FALSE;
      }
   }
   if (result == FALSE) {
      HTS_Model_clear(model);
      return FALSE;
   }
   return TRUE;
}

/* HTS_Model_load: load pdf and tree */
static HTS_Boolean HTS_Model_load(HTS_Model * model, HTS_File * pdf, HTS_File * tree, size_t vector_length, size_t num_windows, HTS_Boolean is_msd)
{
   /* check */
   if (model == NULL || pdf == NULL || vector_length == 0 || num_windows == 0)
      return FALSE;

   /* reset */
   HTS_Model_clear(model);

   /* load tree */
   if (HTS_Model_load_tree(model, tree) != TRUE) {
      HTS_Model_clear(model);
      return FALSE;
   }

   /* load pdf */
   if (HTS_Model_load_pdf(model, pdf, vector_length, num_windows, is_msd) != TRUE) {
      HTS_Model_clear(model);
      return FALSE;
   }

   return TRUE;
}


/* HTS_Model_get_index: get index of tree and PDF */
static void HTS_Model_get_index(HTS_Model * model, size_t state_index, const char *string, size_t * tree_index, size_t * pdf_index)
{
   HTS_Tree *tree;
   HTS_Pattern *pattern;
   HTS_Boolean find;

   (*tree_index) = 2;
   (*pdf_index) = 1;

   if (model->tree == NULL)
      return;

   find = FALSE;
   for (tree = model->tree; tree; tree = tree->next) {
      if (tree->state == state_index) {
         pattern = tree->head;
         if (!pattern)
            find = TRUE;
         for (; pattern; pattern = pattern->next)
            if (HTS_pattern_match(string, pattern->string)) {
               find = TRUE;
               break;
            }
         if (find)
            break;
      }
      (*tree_index)++;
   }

   if (tree != NULL) {
      (*pdf_index) = HTS_Tree_search_node(tree, string);
   } else {
      (*pdf_index) = HTS_Tree_search_node(model->tree, string);
   }
}

/* HTS_ModelSet_initialize: initialize model set */
void HTS_ModelSet_initialize(HTS_ModelSet * ms)
{
   ms->hts_voice_version = NULL;
   ms->sampling_frequency = 0;
   ms->frame_period = 0;
   ms->num_voices = 0;
   ms->num_states = 0;
   ms->num_streams = 0;
   ms->stream_type = NULL;
   ms->fullcontext_format = NULL;
   ms->fullcontext_version = NULL;
   ms->gv_off_context = NULL;
   ms->option = NULL;

   ms->duration = NULL;
   ms->window = NULL;
   ms->stream = NULL;
   ms->gv = NULL;
}

/* HTS_ModelSet_clear: free model set */
void HTS_ModelSet_clear(HTS_ModelSet * ms)
{
   size_t i, j;

   if (ms->hts_voice_version != NULL)
      free(ms->hts_voice_version);
   if (ms->stream_type != NULL)
      free(ms->stream_type);
   if (ms->fullcontext_format != NULL)
      free(ms->fullcontext_format);
   if (ms->fullcontext_version != NULL)
      free(ms->fullcontext_version);
   if (ms->gv_off_context != NULL) {
      HTS_Question_clear(ms->gv_off_context);
      free(ms->gv_off_context);
   }
   if (ms->option != NULL) {
      for (i = 0; i < ms->num_streams; i++)
         if (ms->option[i] != NULL)
            free(ms->option[i]);
      free(ms->option);
   }

   if (ms->duration != NULL) {
      for (i = 0; i < ms->num_voices; i++)
         HTS_Model_clear(&ms->duration[i]);
      free(ms->duration);
   }
   if (ms->window != NULL) {
      for (i = 0; i < ms->num_streams; i++)
         HTS_Window_clear(&ms->window[i]);
      free(ms->window);
   }
   if (ms->stream != NULL) {
      for (i = 0; i < ms->num_voices; i++) {
         for (j = 0; j < ms->num_streams; j++)
            HTS_Model_clear(&ms->stream[i][j]);
         free(ms->stream[i]);
      }
      HTS_free(ms->stream);
   }
   if (ms->gv != NULL) {
      for (i = 0; i < ms->num_voices; i++) {
         for (j = 0; j < ms->num_streams; j++)
            HTS_Model_clear(&ms->gv[i][j]);
         free(ms->gv[i]);
      }
      free(ms->gv);
   }
   HTS_ModelSet_initialize(ms);
}

/* HTS_match_head_string: return true if head of str is equal to pattern */
static HTS_Boolean HTS_match_head_string(const char *str, const char *pattern, size_t * matched_size)
{

   (*matched_size) = 0;
   while (1) {
      if (pattern[(*matched_size)] == '\0')
         return TRUE;
      if (str[(*matched_size)] == '\0')
         return FALSE;
      if (str[(*matched_size)] != pattern[(*matched_size)])
         return FALSE;
      (*matched_size)++;
   }
}

/* HTS_strequal: strcmp wrapper */
static HTS_Boolean HTS_strequal(const char *s1, const char *s2)
{
   if (s1 == NULL && s2 == NULL)
      return TRUE;
   else if (s1 == NULL || s2 == NULL)
      return FALSE;
   else
      return strcmp(s1, s2) == 0 ? TRUE : FALSE;
}

/* HTS_ModelSet_load: load model set */
HTS_Boolean HTS_ModelSet_load(HTS_ModelSet * ms, char **voices, size_t num_voices)
{
   size_t i, j, k, s, e;
   HTS_Boolean error = FALSE;
   HTS_File *fp = NULL;
   char buff1[HTS_MAXBUFLEN];
   char buff2[HTS_MAXBUFLEN];
   size_t matched_size;

   char **stream_type_list = NULL;

   size_t *vector_length = NULL;
   HTS_Boolean *is_msd = NULL;
   size_t *num_windows = NULL;
   HTS_Boolean *use_gv = NULL;

   char *gv_off_context = NULL;

   /* temporary values */
   char *temp_hts_voice_version;
   size_t temp_sampling_frequency;
   size_t temp_frame_period;
   size_t temp_num_states;
   size_t temp_num_streams;
   char *temp_stream_type;
   char *temp_fullcontext_format;
   char *temp_fullcontext_version;

   char *temp_gv_off_context;

   size_t *temp_vector_length;
   HTS_Boolean *temp_is_msd;
   size_t *temp_num_windows;
   HTS_Boolean *temp_use_gv;
   char **temp_option;

   char *temp_duration_pdf;
   char *temp_duration_tree;
   char ***temp_stream_win;
   char **temp_stream_pdf;
   char **temp_stream_tree;
   char **temp_gv_pdf;
   char **temp_gv_tree;

   long start_of_data;
   HTS_File *pdf_fp = NULL;
   HTS_File *tree_fp = NULL;
   HTS_File **win_fp = NULL;
   HTS_File *gv_off_context_fp = NULL;

   HTS_ModelSet_clear(ms);

   if (ms == NULL || voices == NULL || num_voices < 1)
      return FALSE;

   ms->num_voices = num_voices;

   for (i = 0; i < num_voices && error == FALSE; i++) {
      /* open file */
      fp = HTS_fopen_from_fn(voices[i], "rb");
      if (fp == NULL) {
         error = TRUE;
         break;
      }
      /* reset GLOBAL options */
      temp_hts_voice_version = NULL;
      temp_sampling_frequency = 0;
      temp_frame_period = 0;
      temp_num_states = 0;
      temp_num_streams = 0;
      temp_stream_type = NULL;
      temp_fullcontext_format = NULL;
      temp_fullcontext_version = NULL;
      temp_gv_off_context = NULL;
      if (HTS_get_token_from_fp_with_separator(fp, buff1, '\n') != TRUE) {
         error = TRUE;
         break;
      }
      /* load GLOBAL options */
      if (HTS_strequal(buff1, "[GLOBAL]") != TRUE) {
         error = TRUE;
         break;
      }
      while (1) {
         if (HTS_get_token_from_fp_with_separator(fp, buff1, '\n') != TRUE) {
            error = TRUE;
            break;
         }
         if (HTS_strequal(buff1, "[STREAM]") == TRUE) {
            break;
         } else if (HTS_match_head_string(buff1, "HTS_VOICE_VERSION:", &matched_size) == TRUE) {
            if (temp_hts_voice_version != NULL)
               free(temp_hts_voice_version);
            temp_hts_voice_version = HTS_strdup(&buff1[matched_size]);
         } else if (HTS_match_head_string(buff1, "SAMPLING_FREQUENCY:", &matched_size) == TRUE) {
            temp_sampling_frequency = (size_t) atoi(&buff1[matched_size]);
         } else if (HTS_match_head_string(buff1, "FRAME_PERIOD:", &matched_size) == TRUE) {
            temp_frame_period = (size_t) atoi(&buff1[matched_size]);
         } else if (HTS_match_head_string(buff1, "NUM_STATES:", &matched_size) == TRUE) {
            temp_num_states = (size_t) atoi(&buff1[matched_size]);
         } else if (HTS_match_head_string(buff1, "NUM_STREAMS:", &matched_size) == TRUE) {
            temp_num_streams = (size_t) atoi(&buff1[matched_size]);
         } else if (HTS_match_head_string(buff1, "STREAM_TYPE:", &matched_size) == TRUE) {
            if (temp_stream_type != NULL)
               free(temp_stream_type);
            temp_stream_type = HTS_strdup(&buff1[matched_size]);
         } else if (HTS_match_head_string(buff1, "FULLCONTEXT_FORMAT:", &matched_size) == TRUE) {
            if (temp_fullcontext_format != NULL)
               free(temp_fullcontext_format);
            temp_fullcontext_format = HTS_strdup(&buff1[matched_size]);
         } else if (HTS_match_head_string(buff1, "FULLCONTEXT_VERSION:", &matched_size) == TRUE) {
            if (temp_fullcontext_version != NULL)
               free(temp_fullcontext_version);
            temp_fullcontext_version = HTS_strdup(&buff1[matched_size]);
         } else if (HTS_match_head_string(buff1, "GV_OFF_CONTEXT:", &matched_size) == TRUE) {
            if (temp_gv_off_context != NULL)
               free(temp_gv_off_context);
            temp_gv_off_context = HTS_strdup(&buff1[matched_size]);
         } else if (HTS_match_head_string(buff1, "COMMENT:", &matched_size) == TRUE) {
         } else {
            HTS_error(0, "HTS_ModelSet_load: Unknown option %s.\n", buff1);
         }
      }
      /* check GLOBAL options */
      if (i == 0) {
         ms->hts_voice_version = temp_hts_voice_version;
         ms->sampling_frequency = temp_sampling_frequency;
         ms->frame_period = temp_frame_period;
         ms->num_states = temp_num_states;
         ms->num_streams = temp_num_streams;
         ms->stream_type = temp_stream_type;
         ms->fullcontext_format = temp_fullcontext_format;
         ms->fullcontext_version = temp_fullcontext_version;
         gv_off_context = temp_gv_off_context;
      } else {
         if (HTS_strequal(ms->hts_voice_version, temp_hts_voice_version) != TRUE)
            error = TRUE;
         if (ms->sampling_frequency != temp_sampling_frequency)
            error = TRUE;
         if (ms->frame_period != temp_frame_period)
            error = TRUE;
         if (ms->num_states != temp_num_states)
            error = TRUE;
         if (ms->num_streams != temp_num_streams)
            error = TRUE;
         if (HTS_strequal(ms->stream_type, temp_stream_type) != TRUE)
            error = TRUE;
         if (HTS_strequal(ms->fullcontext_format, temp_fullcontext_format) != TRUE)
            error = TRUE;
         if (HTS_strequal(ms->fullcontext_version, temp_fullcontext_version) != TRUE)
            error = TRUE;
         if (HTS_strequal(gv_off_context, temp_gv_off_context) != TRUE)
            error = TRUE;
         if (temp_hts_voice_version != NULL)
            free(temp_hts_voice_version);
         if (temp_stream_type != NULL)
            free(temp_stream_type);
         if (temp_fullcontext_format != NULL)
            free(temp_fullcontext_format);
         if (temp_fullcontext_version != NULL)
            free(temp_fullcontext_version);
         if (temp_gv_off_context != NULL)
            free(temp_gv_off_context);
      }
      /* find stream names */
      if (i == 0) {
         stream_type_list = (char **) HTS_calloc(ms->num_streams, sizeof(char *));
         for (j = 0, matched_size = 0; j < ms->num_streams; j++) {
            if (HTS_get_token_from_string_with_separator(ms->stream_type, &matched_size, buff2, ',') == TRUE) {
               stream_type_list[j] = HTS_strdup(buff2);
            } else {
               stream_type_list[j] = NULL;
               error = TRUE;
            }
         }
      }
      if (error != FALSE) {
         if (fp != NULL) {
            HTS_fclose(fp);
            fp = NULL;
         }
         break;
      }
      /* reset STREAM options */
      temp_vector_length = (size_t *) HTS_calloc(ms->num_streams, sizeof(size_t));
      for (j = 0; j < ms->num_streams; j++)
         temp_vector_length[j] = 0;
      temp_is_msd = (HTS_Boolean *) HTS_calloc(ms->num_streams, sizeof(HTS_Boolean));
      for (j = 0; j < ms->num_streams; j++)
         temp_is_msd[j] = FALSE;
      temp_num_windows = (size_t *) HTS_calloc(ms->num_streams, sizeof(size_t));
      for (j = 0; j < ms->num_streams; j++)
         temp_num_windows[j] = 0;
      temp_use_gv = (HTS_Boolean *) HTS_calloc(ms->num_streams, sizeof(HTS_Boolean));
      for (j = 0; j < ms->num_streams; j++)
         temp_use_gv[j] = FALSE;
      temp_option = (char **) HTS_calloc(ms->num_streams, sizeof(char *));
      for (j = 0; j < ms->num_streams; j++)
         temp_option[j] = NULL;
      /* load STREAM options */
      while (1) {
         if (HTS_get_token_from_fp_with_separator(fp, buff1, '\n') != TRUE) {
            error = TRUE;
            break;
         }
         if (strcmp(buff1, "[POSITION]") == 0) {
            break;
         } else if (HTS_match_head_string(buff1, "VECTOR_LENGTH[", &matched_size) == TRUE) {
            if (HTS_get_token_from_string_with_separator(buff1, &matched_size, buff2, ']') == TRUE) {
               if (buff1[matched_size++] == ':') {
                  for (j = 0; j < ms->num_streams; j++)
                     if (strcmp(stream_type_list[j], buff2) == 0) {
                        temp_vector_length[j] = (size_t) atoi(&buff1[matched_size]);
                        break;
                     }
               }
            }
         } else if (HTS_match_head_string(buff1, "IS_MSD[", &matched_size) == TRUE) {
            if (HTS_get_token_from_string_with_separator(buff1, &matched_size, buff2, ']') == TRUE) {
               if (buff1[matched_size++] == ':') {
                  for (j = 0; j < ms->num_streams; j++)
                     if (strcmp(stream_type_list[j], buff2) == 0) {
                        temp_is_msd[j] = (buff1[matched_size] == '1') ? TRUE : FALSE;
                        break;
                     }
               }
            }
         } else if (HTS_match_head_string(buff1, "NUM_WINDOWS[", &matched_size) == TRUE) {
            if (HTS_get_token_from_string_with_separator(buff1, &matched_size, buff2, ']') == TRUE) {
               if (buff1[matched_size++] == ':') {
                  for (j = 0; j < ms->num_streams; j++)
                     if (strcmp(stream_type_list[j], buff2) == 0) {
                        temp_num_windows[j] = (size_t) atoi(&buff1[matched_size]);
                        break;
                     }
               }
            }
         } else if (HTS_match_head_string(buff1, "USE_GV[", &matched_size) == TRUE) {
            if (HTS_get_token_from_string_with_separator(buff1, &matched_size, buff2, ']') == TRUE) {
               if (buff1[matched_size++] == ':') {
                  for (j = 0; j < ms->num_streams; j++)
                     if (strcmp(stream_type_list[j], buff2) == 0) {
                        temp_use_gv[j] = (buff1[matched_size] == '1') ? TRUE : FALSE;
                        break;
                     }
               }
            }
         } else if (HTS_match_head_string(buff1, "OPTION[", &matched_size) == TRUE) {
            if (HTS_get_token_from_string_with_separator(buff1, &matched_size, buff2, ']') == TRUE) {
               if (buff1[matched_size++] == ':') {
                  for (j = 0; j < ms->num_streams; j++)
                     if (strcmp(stream_type_list[j], buff2) == 0) {
                        if (temp_option[j] != NULL)
                           free(temp_option[j]);
                        temp_option[j] = HTS_strdup(&buff1[matched_size]);
                        break;
                     }
               }
            }
         } else {
            HTS_error(0, "HTS_ModelSet_load: Unknown option %s.\n", buff1);
         }
      }
      /* check STREAM options */
      if (i == 0) {
         vector_length = temp_vector_length;
         is_msd = temp_is_msd;
         num_windows = temp_num_windows;
         use_gv = temp_use_gv;
         ms->option = temp_option;
      } else {
         for (j = 0; j < ms->num_streams; j++)
            if (vector_length[j] != temp_vector_length[j])
               error = TRUE;
         for (j = 0; j < ms->num_streams; j++)
            if (is_msd[j] != is_msd[j])
               error = TRUE;
         for (j = 0; j < ms->num_streams; j++)
            if (num_windows[j] != temp_num_windows[j])
               error = TRUE;
         for (j = 0; j < ms->num_streams; j++)
            if (use_gv[j] != temp_use_gv[j])
               error = TRUE;
         for (j = 0; j < ms->num_streams; j++)
            if (HTS_strequal(ms->option[j], temp_option[j]) != TRUE)
               error = TRUE;
         free(temp_vector_length);
         free(temp_is_msd);
         free(temp_num_windows);
         free(temp_use_gv);
         for (j = 0; j < ms->num_streams; j++)
            if (temp_option[j] != NULL)
               free(temp_option[j]);
         free(temp_option);
      }
      if (error != FALSE) {
         if (fp != NULL) {
            HTS_fclose(fp);
            fp = NULL;
         }
         break;
      }
      /* reset POSITION */
      temp_duration_pdf = NULL;
      temp_duration_tree = NULL;
      temp_stream_win = (char ***) HTS_calloc(ms->num_streams, sizeof(char **));
      for (j = 0; j < ms->num_streams; j++) {
         temp_stream_win[j] = (char **) HTS_calloc(num_windows[j], sizeof(char *));
         for (k = 0; k < num_windows[j]; k++)
            temp_stream_win[j][k] = NULL;
      }
      temp_stream_pdf = (char **) HTS_calloc(ms->num_streams, sizeof(char *));
      for (j = 0; j < ms->num_streams; j++)
         temp_stream_pdf[j] = NULL;
      temp_stream_tree = (char **) HTS_calloc(ms->num_streams, sizeof(char *));
      for (j = 0; j < ms->num_streams; j++)
         temp_stream_tree[j] = NULL;
      temp_gv_pdf = (char **) HTS_calloc(ms->num_streams, sizeof(char *));
      for (j = 0; j < ms->num_streams; j++)
         temp_gv_pdf[j] = NULL;
      temp_gv_tree = (char **) HTS_calloc(ms->num_streams, sizeof(char *));
      for (j = 0; j < ms->num_streams; j++)
         temp_gv_tree[j] = NULL;
      /* load POSITION */
      while (1) {
         if (HTS_get_token_from_fp_with_separator(fp, buff1, '\n') != TRUE) {
            error = TRUE;
            break;
         }
         if (strcmp(buff1, "[DATA]") == 0) {
            break;
         } else if (HTS_match_head_string(buff1, "DURATION_PDF:", &matched_size) == TRUE) {
            if (temp_duration_pdf != NULL)
               free(temp_duration_pdf);
            temp_duration_pdf = HTS_strdup(&buff1[matched_size]);
         } else if (HTS_match_head_string(buff1, "DURATION_TREE:", &matched_size) == TRUE) {
            if (temp_duration_tree != NULL)
               free(temp_duration_tree);
            temp_duration_tree = HTS_strdup(&buff1[matched_size]);
         } else if (HTS_match_head_string(buff1, "STREAM_WIN[", &matched_size) == TRUE) {
            if (HTS_get_token_from_string_with_separator(buff1, &matched_size, buff2, ']') == TRUE) {
               if (buff1[matched_size++] == ':') {
                  for (j = 0; j < ms->num_streams; j++) {
                     if (strcmp(stream_type_list[j], buff2) == 0) {
                        for (k = 0; k < num_windows[j]; k++) {
                           if (HTS_get_token_from_string_with_separator(buff1, &matched_size, buff2, ',') == TRUE)
                              temp_stream_win[j][k] = HTS_strdup(buff2);
                           else
                              error = TRUE;
                        }
                        break;
                     }
                  }
               }
            }
         } else if (HTS_match_head_string(buff1, "STREAM_PDF[", &matched_size) == TRUE) {
            if (HTS_get_token_from_string_with_separator(buff1, &matched_size, buff2, ']') == TRUE) {
               if (buff1[matched_size++] == ':') {
                  for (j = 0; j < ms->num_streams; j++) {
                     if (strcmp(stream_type_list[j], buff2) == 0) {
                        if (temp_stream_pdf[j] != NULL)
                           free(temp_stream_pdf[j]);
                        temp_stream_pdf[j] = HTS_strdup(&buff1[matched_size]);
                        break;
                     }
                  }
               }
            }
         } else if (HTS_match_head_string(buff1, "STREAM_TREE[", &matched_size) == TRUE) {
            if (HTS_get_token_from_string_with_separator(buff1, &matched_size, buff2, ']') == TRUE) {
               if (buff1[matched_size++] == ':') {
                  for (j = 0; j < ms->num_streams; j++) {
                     if (strcmp(stream_type_list[j], buff2) == 0) {
                        if (temp_stream_tree[j] != NULL)
                           free(temp_stream_tree[j]);
                        temp_stream_tree[j] = HTS_strdup(&buff1[matched_size]);
                        break;
                     }
                  }
               }
            }
         } else if (HTS_match_head_string(buff1, "GV_PDF[", &matched_size) == TRUE) {
            if (HTS_get_token_from_string_with_separator(buff1, &matched_size, buff2, ']') == TRUE) {
               if (buff1[matched_size++] == ':') {
                  for (j = 0; j < ms->num_streams; j++) {
                     if (strcmp(stream_type_list[j], buff2) == 0) {
                        if (temp_gv_pdf[j] != NULL)
                           free(temp_gv_pdf[j]);
                        temp_gv_pdf[j] = HTS_strdup(&buff1[matched_size]);
                        break;
                     }
                  }
               }
            }
         } else if (HTS_match_head_string(buff1, "GV_TREE[", &matched_size) == TRUE) {
            if (HTS_get_token_from_string_with_separator(buff1, &matched_size, buff2, ']') == TRUE) {
               if (buff1[matched_size++] == ':') {
                  for (j = 0; j < ms->num_streams; j++) {
                     if (strcmp(stream_type_list[j], buff2) == 0) {
                        if (temp_gv_tree[j] != NULL)
                           free(temp_gv_tree[j]);
                        temp_gv_tree[j] = HTS_strdup(&buff1[matched_size]);
                        break;
                     }
                  }
               }
            }
         } else {
            HTS_error(0, "HTS_ModelSet_load: Unknown option %s.\n", buff1);
         }
      }
      /* check POSITION */
      if (temp_duration_pdf == NULL)
         error = TRUE;
      for (j = 0; j < ms->num_streams; j++)
         for (k = 0; k < num_windows[j]; k++)
            if (temp_stream_win[j][k] == NULL)
               error = TRUE;
      for (j = 0; j < ms->num_streams; j++)
         if (temp_stream_pdf[j] == NULL)
            error = TRUE;
      /* prepare memory */
      if (i == 0) {
         ms->duration = (HTS_Model *) HTS_calloc(num_voices, sizeof(HTS_Model));
         for (j = 0; j < num_voices; j++)
            HTS_Model_initialize(&ms->duration[j]);
         ms->window = (HTS_Window *) HTS_calloc(ms->num_streams, sizeof(HTS_Window));
         for (j = 0; j < ms->num_streams; j++)
            HTS_Window_initialize(&ms->window[j]);
         ms->stream = (HTS_Model **) HTS_calloc(num_voices, sizeof(HTS_Model *));
         for (j = 0; j < num_voices; j++) {
            ms->stream[j] = (HTS_Model *) HTS_calloc(ms->num_streams, sizeof(HTS_Model));
            for (k = 0; k < ms->num_streams; k++)
               HTS_Model_initialize(&ms->stream[j][k]);
         }
         ms->gv = (HTS_Model **) HTS_calloc(num_voices, sizeof(HTS_Model *));
         for (j = 0; j < num_voices; j++) {
            ms->gv[j] = (HTS_Model *) HTS_calloc(ms->num_streams, sizeof(HTS_Model));
            for (k = 0; k < ms->num_streams; k++)
               HTS_Model_initialize(&ms->gv[j][k]);
         }
      }
      start_of_data = HTS_ftell(fp);
      /* load duration */
      pdf_fp = NULL;
      tree_fp = NULL;
      matched_size = 0;
      if (HTS_get_token_from_string_with_separator(temp_duration_pdf, &matched_size, buff2, '-') == TRUE) {
         s = (size_t) atoi(buff2);
         e = (size_t) atoi(&temp_duration_pdf[matched_size]);
         HTS_fseek(fp, (long) s, SEEK_CUR);
         pdf_fp = HTS_fopen_from_fp(fp, e - s + 1);
         HTS_fseek(fp, start_of_data, SEEK_SET);
      }
      matched_size = 0;
      if (HTS_get_token_from_string_with_separator(temp_duration_tree, &matched_size, buff2, '-') == TRUE) {
         s = (size_t) atoi(buff2);
         e = (size_t) atoi(&temp_duration_tree[matched_size]);
         HTS_fseek(fp, (long) s, SEEK_CUR);
         tree_fp = HTS_fopen_from_fp(fp, e - s + 1);
         HTS_fseek(fp, start_of_data, SEEK_SET);
      }
      if (HTS_Model_load(&ms->duration[i], pdf_fp, tree_fp, ms->num_states, 1, FALSE) != TRUE)
         error = TRUE;
      HTS_fclose(pdf_fp);
      HTS_fclose(tree_fp);
      /* load windows */
      for (j = 0; j < ms->num_streams; j++) {
         win_fp = (HTS_File **) HTS_calloc(num_windows[j], sizeof(HTS_File *));
         for (k = 0; k < num_windows[j]; k++)
            win_fp[k] = NULL;
         for (k = 0; k < num_windows[j]; k++) {
            matched_size = 0;
            if (HTS_get_token_from_string_with_separator(temp_stream_win[j][k], &matched_size, buff2, '-') == TRUE) {
               s = (size_t) atoi(buff2);
               e = (size_t) atoi(&temp_stream_win[j][k][matched_size]);
               HTS_fseek(fp, (long) s, SEEK_CUR);
               win_fp[k] = HTS_fopen_from_fp(fp, e - s + 1);
               HTS_fseek(fp, start_of_data, SEEK_SET);
            }
         }
         HTS_Window_clear(&ms->window[j]);      /* if windows were loaded already, release them */
         if (HTS_Window_load(&ms->window[j], win_fp, num_windows[j]) != TRUE)
            error = TRUE;
         for (k = 0; k < num_windows[j]; k++)
            HTS_fclose(win_fp[k]);
         free(win_fp);
      }
      /* load streams */
      for (j = 0; j < ms->num_streams; j++) {
         pdf_fp = NULL;
         tree_fp = NULL;
         matched_size = 0;
         if (HTS_get_token_from_string_with_separator(temp_stream_pdf[j], &matched_size, buff2, '-') == TRUE) {
            s = (size_t) atoi(buff2);
            e = (size_t) atoi(&temp_stream_pdf[j][matched_size]);
            HTS_fseek(fp, (long) s, SEEK_CUR);
            pdf_fp = HTS_fopen_from_fp(fp, e - s + 1);
            HTS_fseek(fp, start_of_data, SEEK_SET);
         }
         matched_size = 0;
         if (HTS_get_token_from_string_with_separator(temp_stream_tree[j], &matched_size, buff2, '-') == TRUE) {
            s = (size_t) atoi(buff2);
            e = (size_t) atoi(&temp_stream_tree[j][matched_size]);
            HTS_fseek(fp, (long) s, SEEK_CUR);
            tree_fp = HTS_fopen_from_fp(fp, e - s + 1);
            HTS_fseek(fp, start_of_data, SEEK_SET);
         }
         if (HTS_Model_load(&ms->stream[i][j], pdf_fp, tree_fp, vector_length[j], num_windows[j], is_msd[j]) != TRUE)
            error = TRUE;
         HTS_fclose(pdf_fp);
         HTS_fclose(tree_fp);
      }
      /* load GVs */
      for (j = 0; j < ms->num_streams; j++) {
         pdf_fp = NULL;
         tree_fp = NULL;
         matched_size = 0;
         if (HTS_get_token_from_string_with_separator(temp_gv_pdf[j], &matched_size, buff2, '-') == TRUE) {
            s = (size_t) atoi(buff2);
            e = (size_t) atoi(&temp_gv_pdf[j][matched_size]);
            HTS_fseek(fp, (long) s, SEEK_CUR);
            pdf_fp = HTS_fopen_from_fp(fp, e - s + 1);
            HTS_fseek(fp, start_of_data, SEEK_SET);
         }
         matched_size = 0;
         if (HTS_get_token_from_string_with_separator(temp_gv_tree[j], &matched_size, buff2, '-') == TRUE) {
            s = (size_t) atoi(buff2);
            e = (size_t) atoi(&temp_gv_tree[j][matched_size]);
            HTS_fseek(fp, (long) s, SEEK_CUR);
            tree_fp = HTS_fopen_from_fp(fp, e - s + 1);
            HTS_fseek(fp, start_of_data, SEEK_SET);
         }
         if (use_gv[j] == TRUE) {
            if (HTS_Model_load(&ms->gv[i][j], pdf_fp, tree_fp, vector_length[j], 1, FALSE) != TRUE)
               error = TRUE;
         }
         HTS_fclose(pdf_fp);
         HTS_fclose(tree_fp);
      }
      /* free */
      if (temp_duration_pdf != NULL)
         free(temp_duration_pdf);
      if (temp_duration_tree != NULL)
         free(temp_duration_tree);
      for (j = 0; j < ms->num_streams; j++) {
         for (k = 0; k < num_windows[j]; k++)
            if (temp_stream_win[j][k] != NULL)
               free(temp_stream_win[j][k]);
         free(temp_stream_win[j]);
      }
      free(temp_stream_win);
      for (j = 0; j < ms->num_streams; j++)
         if (temp_stream_pdf[j] != NULL)
            free(temp_stream_pdf[j]);
      free(temp_stream_pdf);
      for (j = 0; j < ms->num_streams; j++)
         if (temp_stream_tree[j] != NULL)
            free(temp_stream_tree[j]);
      free(temp_stream_tree);
      for (j = 0; j < ms->num_streams; j++)
         if (temp_gv_pdf[j] != NULL)
            free(temp_gv_pdf[j]);
      free(temp_gv_pdf);
      for (j = 0; j < ms->num_streams; j++)
         if (temp_gv_tree[j] != NULL)
            free(temp_gv_tree[j]);
      free(temp_gv_tree);
      /* fclose */
      if (fp != NULL) {
         HTS_fclose(fp);
         fp = NULL;
      }
      if (error != FALSE)
         break;
   }

   if (gv_off_context != NULL) {
      sprintf(buff1, "GV-Off { %s }", gv_off_context);
      gv_off_context_fp = HTS_fopen_from_data((void *) buff1, strlen(buff1) + 1);
      ms->gv_off_context = (HTS_Question *) HTS_calloc(1, sizeof(HTS_Question));
      HTS_Question_initialize(ms->gv_off_context);
      HTS_Question_load(ms->gv_off_context, gv_off_context_fp);
      HTS_fclose(gv_off_context_fp);
      free(gv_off_context);
   }

   if (stream_type_list != NULL) {
      for (i = 0; i < ms->num_streams; i++)
         if (stream_type_list[i] != NULL)
            free(stream_type_list[i]);
      free(stream_type_list);
   }

   if (vector_length != NULL)
      free(vector_length);
   if (is_msd != NULL)
      free(is_msd);
   if (num_windows != NULL)
      free(num_windows);
   if (use_gv != NULL)
      free(use_gv);

   return !error;
}

/* HTS_ModelSet_get_sampling_frequency: get sampling frequency of HTS voices */
size_t HTS_ModelSet_get_sampling_frequency(HTS_ModelSet * ms)
{
   return ms->sampling_frequency;
}

/* HTS_ModelSet_get_fperiod: get frame period of HTS voices */
size_t HTS_ModelSet_get_fperiod(HTS_ModelSet * ms)
{
   return ms->frame_period;
}

/* HTS_ModelSet_get_fperiod: get stream option */
const char *HTS_ModelSet_get_option(HTS_ModelSet * ms, size_t stream_index)
{
   return ms->option[stream_index];
}

/* HTS_ModelSet_get_gv_flag: get GV flag */
HTS_Boolean HTS_ModelSet_get_gv_flag(HTS_ModelSet * ms, const char *string)
{
   if (ms->gv_off_context == NULL)
      return TRUE;
   else if (HTS_Question_match(ms->gv_off_context, string) == TRUE)
      return FALSE;
   else
      return TRUE;
}

/* HTS_ModelSet_get_nstate: get number of state */
size_t HTS_ModelSet_get_nstate(HTS_ModelSet * ms)
{
   return ms->num_states;
}

/* HTS_Engine_get_fullcontext_label_format: get full-context label format */
const char *HTS_ModelSet_get_fullcontext_label_format(HTS_ModelSet * ms)
{
   return ms->fullcontext_format;
}

/* HTS_Engine_get_fullcontext_label_version: get full-context label version */
const char *HTS_ModelSet_get_fullcontext_label_version(HTS_ModelSet * ms)
{
   return ms->fullcontext_version;
}

/* HTS_ModelSet_get_nstream: get number of stream */
size_t HTS_ModelSet_get_nstream(HTS_ModelSet * ms)
{
   return ms->num_streams;
}

/* HTS_ModelSet_get_nvoices: get number of stream */
size_t HTS_ModelSet_get_nvoices(HTS_ModelSet * ms)
{
   return ms->num_voices;
}

/* HTS_ModelSet_get_vector_length: get vector length */
size_t HTS_ModelSet_get_vector_length(HTS_ModelSet * ms, size_t stream_index)
{
   return ms->stream[0][stream_index].vector_length;
}

/* HTS_ModelSet_is_msd: get MSD flag */
HTS_Boolean HTS_ModelSet_is_msd(HTS_ModelSet * ms, size_t stream_index)
{
   return ms->stream[0][stream_index].is_msd;
}

/* HTS_ModelSet_get_window_size: get dynamic window size */
size_t HTS_ModelSet_get_window_size(HTS_ModelSet * ms, size_t stream_index)
{
   return ms->window[stream_index].size;
}

/* HTS_ModelSet_get_window_left_width: get left width of dynamic window */
int HTS_ModelSet_get_window_left_width(HTS_ModelSet * ms, size_t stream_index, size_t window_index)
{
   return ms->window[stream_index].l_width[window_index];
}

/* HTS_ModelSet_get_window_right_width: get right width of dynamic window */
int HTS_ModelSet_get_window_right_width(HTS_ModelSet * ms, size_t stream_index, size_t window_index)
{
   return ms->window[stream_index].r_width[window_index];
}

/* HTS_ModelSet_get_window_coefficient: get coefficient of dynamic window */
double HTS_ModelSet_get_window_coefficient(HTS_ModelSet * ms, size_t stream_index, size_t window_index, size_t coefficient_index)
{
   return ms->window[stream_index].coefficient[window_index][coefficient_index];
}

/* HTS_ModelSet_get_window_max_width: get max width of dynamic window */
size_t HTS_ModelSet_get_window_max_width(HTS_ModelSet * ms, size_t stream_index)
{
   return ms->window[stream_index].max_width;
}

/* HTS_ModelSet_use_gv: get GV flag */
HTS_Boolean HTS_ModelSet_use_gv(HTS_ModelSet * ms, size_t stream_index)
{
   if (ms->gv[0][stream_index].vector_length != 0)
      return TRUE;
   else
      return FALSE;
}

/* HTS_Model_add_parameter: get parameter using interpolation weight */
static void HTS_Model_add_parameter(HTS_Model * model, size_t state_index, const char *string, double *mean, double *vari, double *msd, double weight)
{
   size_t i;
   size_t tree_index, pdf_index;
   size_t len = model->vector_length * model->num_windows;

   HTS_Model_get_index(model, state_index, string, &tree_index, &pdf_index);
   for (i = 0; i < len; i++) {
      mean[i] += weight * model->pdf[tree_index][pdf_index][i];
      vari[i] += weight * model->pdf[tree_index][pdf_index][i + len];
   }
   if (msd != NULL && model->is_msd == TRUE)
      *msd += weight * model->pdf[tree_index][pdf_index][len + len];
}

/* HTS_ModelSet_get_duration_index: get duration PDF & tree index */
void HTS_ModelSet_get_duration_index(HTS_ModelSet * ms, size_t voice_index, const char *string, size_t * tree_index, size_t * pdf_index)
{
   HTS_Model_get_index(&ms->duration[voice_index], 2, string, tree_index, pdf_index);
}

/* HTS_ModelSet_get_duration: get duration using interpolation weight */
void HTS_ModelSet_get_duration(HTS_ModelSet * ms, const char *string, const double *iw, double *mean, double *vari)
{
   size_t i;
   size_t len = ms->num_states;

   for (i = 0; i < len; i++) {
      mean[i] = 0.0;
      vari[i] = 0.0;
   }
   for (i = 0; i < ms->num_voices; i++)
      if (iw[i] != 0.0)
         HTS_Model_add_parameter(&ms->duration[i], 2, string, mean, vari, NULL, iw[i]);
}

/* HTS_ModelSet_get_parameter_index: get paramter PDF & tree index */
void HTS_ModelSet_get_parameter_index(HTS_ModelSet * ms, size_t voice_index, size_t stream_index, size_t state_index, const char *string, size_t * tree_index, size_t * pdf_index)
{
   HTS_Model_get_index(&ms->stream[voice_index][stream_index], state_index, string, tree_index, pdf_index);
}

/* HTS_ModelSet_get_parameter: get parameter using interpolation weight */
void HTS_ModelSet_get_parameter(HTS_ModelSet * ms, size_t stream_index, size_t state_index, const char *string, const double *const *iw, double *mean, double *vari, double *msd)
{
   size_t i;
   size_t len = ms->stream[0][stream_index].vector_length * ms->stream[0][stream_index].num_windows;

   for (i = 0; i < len; i++) {
      mean[i] = 0.0;
      vari[i] = 0.0;
   }
   if (msd != NULL)
      *msd = 0.0;

   for (i = 0; i < ms->num_voices; i++)
      if (iw[i][stream_index] != 0.0)
         HTS_Model_add_parameter(&ms->stream[i][stream_index], state_index, string, mean, vari, msd, iw[i][stream_index]);
}

/* HTS_ModelSet_get_gv_index: get gv PDF & tree index */
void HTS_ModelSet_get_gv_index(HTS_ModelSet * ms, size_t voice_index, size_t stream_index, const char *string, size_t * tree_index, size_t * pdf_index)
{
   HTS_Model_get_index(&ms->gv[voice_index][stream_index], 2, string, tree_index, pdf_index);
}

/* HTS_ModelSet_get_gv: get GV using interpolation weight */
void HTS_ModelSet_get_gv(HTS_ModelSet * ms, size_t stream_index, const char *string, const double *const *iw, double *mean, double *vari)
{
   size_t i;
   size_t len = ms->stream[0][stream_index].vector_length;

   for (i = 0; i < len; i++) {
      mean[i] = 0.0;
      vari[i] = 0.0;
   }
   for (i = 0; i < ms->num_voices; i++)
      if (iw[i][stream_index] != 0.0)
         HTS_Model_add_parameter(&ms->gv[i][stream_index], 2, string, mean, vari, NULL, iw[i][stream_index]);
}

HTS_MODEL_C_END;

#endif                          /* !HTS_MODEL_C */
