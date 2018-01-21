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

#ifndef HTS106_MODEL_C
#define HTS106_MODEL_C

#ifdef __cplusplus
#define HTS106_MODEL_C_START extern "C" {
#define HTS106_MODEL_C_END   }
#else
#define HTS106_MODEL_C_START
#define HTS106_MODEL_C_END
#endif                          /* __CPLUSPLUS */

HTS106_MODEL_C_START;

#include <stdlib.h>             /* for atoi(),abs() */
#include <string.h>             /* for strlen(),strstr(),strrchr(),strcmp() */
#include <ctype.h>              /* for isdigit() */

/* hts_engine libraries */
#include "HTS106_hidden.h"

/* HTS106_dp_match: recursive matching */
static HTS106_Boolean HTS106_dp_match(const char *string, const char *pattern, const int pos, const int max)
{
   if (pos > max)
      return FALSE;
   if (string[0] == '\0' && pattern[0] == '\0')
      return TRUE;
   if (pattern[0] == '*') {
      if (HTS106_dp_match(string + 1, pattern, pos + 1, max) == 1)
         return TRUE;
      else
         return HTS106_dp_match(string, pattern + 1, pos, max);
   }
   if (string[0] == pattern[0] || pattern[0] == '?') {
      if (HTS106_dp_match(string + 1, pattern + 1, pos + 1, max + 1) == 1)
         return TRUE;
   }

   return FALSE;
}

/* HTS106_pattern_match: pattern matching function */
static HTS106_Boolean HTS106_pattern_match(const char *string, const char *pattern)
{
   int i, j;
   int buff_length, max = 0, nstar = 0, nquestion = 0;
   char buff[HTS106_MAXBUFLEN];
   const int pattern_length = strlen(pattern);

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
      return HTS106_dp_match(string, pattern, 0, (int) (strlen(string) - max));
}

/* HTS106_is_num: check given buffer is number or not */
static HTS106_Boolean HTS106_is_num(const char *buff)
{
   int i;
   const int length = (int) strlen(buff);

   for (i = 0; i < length; i++)
      if (!(isdigit((int) buff[i]) || (buff[i] == '-')))
         return FALSE;

   return TRUE;
}

/* HTS106_name2num: convert name of node to number */
static int HTS106_name2num(const char *buff)
{
   int i;

   for (i = strlen(buff) - 1; '0' <= buff[i] && buff[i] <= '9' && i >= 0; i--);
   i++;

   return atoi(&buff[i]);
}

/* HTS106_get_state_num: return the number of state */
static int HTS106_get_state_num(char *string)
{
   char *left, *right;

   if (((left = strchr(string, '[')) == NULL)
       || ((right = strrchr(string, ']')) == NULL))
      return 0;
   *right = '\0';
   string = left + 1;

   return atoi(string);
}

static void HTS106_Question_clear(HTS106_Question * question);

/* HTS106_Question_load: Load questions from file */
static HTS106_Boolean HTS106_Question_load(HTS106_Question * question, HTS106_File * fp)
{
   char buff[HTS106_MAXBUFLEN];
   HTS106_Pattern *pattern, *last_pattern;

   if (question == NULL || fp == NULL)
      return FALSE;

   /* get question name */
   if (HTS106_get_pattern_token(fp, buff) == FALSE)
      return FALSE;
   question->string = HTS106_strdup(buff);
   question->head = NULL;
   /* get pattern list */
   if (HTS106_get_pattern_token(fp, buff) == FALSE) {
      free(question->string);
      question->string = NULL;
      return FALSE;
   }
   last_pattern = NULL;
   if (strcmp(buff, "{") == 0) {
      while (1) {
         if (HTS106_get_pattern_token(fp, buff) == FALSE) {
            HTS106_Question_clear(question);
            return FALSE;
         }
         pattern = (HTS106_Pattern *) HTS106_calloc(1, sizeof(HTS106_Pattern));
         if (question->head)
            last_pattern->next = pattern;
         else                   /* first time */
            question->head = pattern;
         pattern->string = HTS106_strdup(buff);
         pattern->next = NULL;
         if (HTS106_get_pattern_token(fp, buff) == FALSE) {
            HTS106_Question_clear(question);
            return FALSE;
         }
         if (!strcmp(buff, "}"))
            break;
         last_pattern = pattern;
      }
   }
   return TRUE;
}

/* HTS106_Question_match: check given string match given question */
static HTS106_Boolean HTS106_Question_match(const HTS106_Question * question, const char *string, const RHVoice_parsed_label_string* parsed)
{
   HTS106_Pattern *pattern;

   for (pattern = question->head; pattern; pattern = pattern->next) {
     if(parsed!=NULL) {
       if(RHVoice_question_match(parsed, pattern->string))
         return TRUE;
}
     else if (HTS106_pattern_match(string, pattern->string))
         return TRUE;
   }

   return FALSE;
}

/* HTS106_Question_find_question: find question from question list */
static HTS106_Question *HTS106_Question_find_question(HTS106_Question * question, const char *buff)
{

   for (; question; question = question->next)
      if (strcmp(buff, question->string) == 0)
         return question;

   HTS106_error(1, "HTS106_Question_find_question: Cannot find question %s.\n", buff);
   return NULL;                 /* make compiler happy */
}

/* HTS106_Question_clear: clear loaded question */
static void HTS106_Question_clear(HTS106_Question * question)
{
   HTS106_Pattern *pattern, *next_pattern;

   HTS106_free(question->string);
   for (pattern = question->head; pattern; pattern = next_pattern) {
      next_pattern = pattern->next;
      HTS106_free(pattern->string);
      HTS106_free(pattern);
   }
}

/* HTS106_Node_find: find node for given number */
static HTS106_Node *HTS106_Node_find(HTS106_Node * node, const int num)
{
   for (; node; node = node->next)
      if (node->index == num)
         return node;

   HTS106_error(1, "HTS106_Node_find: Cannot find node %d.\n", num);
   return NULL;                 /* make compiler happy */
}

/* HTS106_Node_clear: recursive function to free Node */
static void HTS106_Node_clear(HTS106_Node * node)
{
   if (node->yes != NULL)
      HTS106_Node_clear(node->yes);
   if (node->no != NULL)
      HTS106_Node_clear(node->no);
   HTS106_free(node);
}

/* HTS106_Tree_parse_pattern: parse pattern specified for each tree */
static void HTS106_Tree_parse_pattern(HTS106_Tree * tree, char *string)
{
   char *left, *right;
   HTS106_Pattern *pattern, *last_pattern;

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
         pattern = (HTS106_Pattern *) HTS106_calloc(1, sizeof(HTS106_Pattern));
         if (tree->head) {
            last_pattern->next = pattern;
         } else {
            tree->head = pattern;
         }
         *left = '\0';
         pattern->string = HTS106_strdup(string);
         string = left + 1;
         pattern->next = NULL;
         last_pattern = pattern;
      }
   }
}

static void HTS106_Tree_clear(HTS106_Tree * tree);

/* HTS106_Tree_load: Load trees */
static HTS106_Boolean HTS106_Tree_load(HTS106_Tree * tree, HTS106_File * fp, HTS106_Question * question)
{
   char buff[HTS106_MAXBUFLEN];
   HTS106_Node *node, *last_node;

   if (tree == NULL || fp == NULL)
      return FALSE;

   if (HTS106_get_pattern_token(fp, buff) == FALSE) {
      HTS106_Tree_clear(tree);
      return FALSE;
   }
   node = (HTS106_Node *) HTS106_calloc(1, sizeof(HTS106_Node));
   node->index = 0;
   tree->root = last_node = node;

   if (strcmp(buff, "{") == 0) {
      while (HTS106_get_pattern_token(fp, buff) == TRUE && strcmp(buff, "}") != 0) {
         node = HTS106_Node_find(last_node, atoi(buff));
         if (node == NULL) {
            HTS106_Tree_clear(tree);
            return FALSE;
         }
         if (HTS106_get_pattern_token(fp, buff) == FALSE) {
            HTS106_Tree_clear(tree);
            return FALSE;
         }
         node->quest = HTS106_Question_find_question(question, buff);
         if (node->quest == NULL) {
            HTS106_Tree_clear(tree);
            return FALSE;
         }
         node->yes = (HTS106_Node *) HTS106_calloc(1, sizeof(HTS106_Node));
         node->no = (HTS106_Node *) HTS106_calloc(1, sizeof(HTS106_Node));

         if (HTS106_get_pattern_token(fp, buff) == FALSE) {
            node->quest = NULL;
            free(node->yes);
            free(node->no);
            HTS106_Tree_clear(tree);
            return FALSE;
         }
         if (HTS106_is_num(buff))
            node->no->index = atoi(buff);
         else
            node->no->pdf = HTS106_name2num(buff);
         node->no->next = last_node;
         last_node = node->no;

         if (HTS106_get_pattern_token(fp, buff) == FALSE) {
            node->quest = NULL;
            free(node->yes);
            free(node->no);
            HTS106_Tree_clear(tree);
            return FALSE;
         }
         if (HTS106_is_num(buff))
            node->yes->index = atoi(buff);
         else
            node->yes->pdf = HTS106_name2num(buff);
         node->yes->next = last_node;
         last_node = node->yes;
      }
   } else {
      node->pdf = HTS106_name2num(buff);
   }

   return TRUE;
}

/* HTS106_Node_search: tree search */
static int HTS106_Tree_search_node(HTS106_Tree * tree, const char *string, const RHVoice_parsed_label_string* parsed)
{
   HTS106_Node *node = tree->root;

   while (node != NULL) {
      if (node->quest == NULL)
         return node->pdf;
      if (HTS106_Question_match(node->quest, string, parsed)) {
         if (node->yes->pdf > 0)
            return node->yes->pdf;
         node = node->yes;
      } else {
         if (node->no->pdf > 0)
            return node->no->pdf;
         node = node->no;
      }
   }

   HTS106_error(1, "HTS106_Tree_search_node: Cannot find node.\n");
   return -1;                   /* make compiler happy */
}

/* HTS106_Tree_clear: clear given tree */
static void HTS106_Tree_clear(HTS106_Tree * tree)
{
   HTS106_Pattern *pattern, *next_pattern;

   for (pattern = tree->head; pattern; pattern = next_pattern) {
      next_pattern = pattern->next;
      HTS106_free(pattern->string);
      HTS106_free(pattern);
   }

   HTS106_Node_clear(tree->root);
}

/* HTS106_Window_initialize: initialize dynamic window */
static void HTS106_Window_initialize(HTS106_Window * win)
{
   win->size = 0;
   win->l_width = NULL;
   win->r_width = NULL;
   win->coefficient = NULL;
   win->max_width = 0;
}

static void HTS106_Window_clear(HTS106_Window * win);

/* HTS106_Window_load: load dynamic windows */
static HTS106_Boolean HTS106_Window_load(HTS106_Window * win, HTS106_File ** fp, int size)
{
   int i, j;
   int fsize, length;
   char buff[HTS106_MAXBUFLEN];
   HTS106_Boolean result = TRUE;

   /* check */
   if (win == NULL || fp == NULL || size <= 0)
      return FALSE;

   win->size = size;
   win->l_width = (int *) HTS106_calloc(win->size, sizeof(int));
   win->r_width = (int *) HTS106_calloc(win->size, sizeof(int));
   win->coefficient = (double **) HTS106_calloc(win->size, sizeof(double *));
   /* set delta coefficents */
   for (i = 0; i < win->size; i++) {
      if (HTS106_get_token(fp[i], buff) == FALSE) {
         result = FALSE;
         fsize = 1;
      } else {
         fsize = atoi(buff);
         if (fsize <= 0) {
            result = FALSE;
            fsize = 1;
         }
      }
      /* read coefficients */
      win->coefficient[i] = (double *) HTS106_calloc(fsize, sizeof(double));
      for (j = 0; j < fsize; j++) {
         if (HTS106_get_token(fp[i], buff) == FALSE) {
            result = FALSE;
            win->coefficient[i][j] = 0.0;
         } else {
            win->coefficient[i][j] = (double) atof(buff);
         }
      }
      /* set pointer */
      length = fsize / 2;
      win->coefficient[i] += length;
      win->l_width[i] = -length;
      win->r_width[i] = length;
      if (fsize % 2 == 0)
         win->r_width[i]--;
   }
   /* calcurate max_width to determine size of band matrix */
   win->max_width = 0;
   for (i = 0; i < win->size; i++) {
      if (win->max_width < abs(win->l_width[i]))
         win->max_width = abs(win->l_width[i]);
      if (win->max_width < abs(win->r_width[i]))
         win->max_width = abs(win->r_width[i]);
   }

   if (result == FALSE) {
      HTS106_Window_clear(win);
      return FALSE;
   }
   return TRUE;
}

/* HTS106_Window_clear: free dynamic window */
static void HTS106_Window_clear(HTS106_Window * win)
{
   int i;

   if (win->coefficient) {
      for (i = win->size - 1; i >= 0; i--) {
         win->coefficient[i] += win->l_width[i];
         HTS106_free(win->coefficient[i]);
      }
      HTS106_free(win->coefficient);
   }
   if (win->l_width)
      HTS106_free(win->l_width);
   if (win->r_width)
      HTS106_free(win->r_width);

   HTS106_Window_initialize(win);
}

/* HTS106_Model_initialize: initialize model */
static void HTS106_Model_initialize(HTS106_Model * model)
{
   model->vector_length = 0;
   model->ntree = 0;
   model->npdf = NULL;
   model->pdf = NULL;
   model->tree = NULL;
   model->question = NULL;
}

static void HTS106_Model_clear(HTS106_Model * model);

/* HTS106_Model_load_pdf: load pdfs */
static HTS106_Boolean HTS106_Model_load_pdf(HTS106_Model * model, HTS106_File * fp, int ntree, HTS106_Boolean msd_flag)
{
   int i, j, k, l, m;
   float temp;
   int ssize;
   HTS106_Boolean result = TRUE;

   /* check */
   if (model == NULL || fp == NULL || ntree <= 0) {
      HTS106_error(1, "HTS106_Model_load_pdf: File for pdfs is not specified.\n");
      return FALSE;
   }

   /* load pdf */
   model->ntree = ntree;
   /* read MSD flag */
   HTS106_fread_big_endian(&i, sizeof(int), 1, fp);
   if ((i != 0 || msd_flag != FALSE) && (i != 1 || msd_flag != TRUE)) {
      HTS106_error(1, "HTS106_Model_load_pdf: Failed to load header of pdfs.\n");
      HTS106_Model_initialize(model);
      return FALSE;
   }
   /* read stream size */
   HTS106_fread_big_endian(&ssize, sizeof(int), 1, fp);
   if (ssize < 1) {
      HTS106_error(1, "HTS106_Model_load_pdf: Failed to load header of pdfs.\n");
      HTS106_Model_initialize(model);
      return FALSE;
   }
   /* read vector size */
   HTS106_fread_big_endian(&model->vector_length, sizeof(int), 1, fp);
   if (model->vector_length <= 0) {
      HTS106_error(1, "HTS106_Model_load_pdf: # of HMM states %d should be positive.\n", model->vector_length);
      HTS106_Model_initialize(model);
      return FALSE;
   }
   model->npdf = (int *) HTS106_calloc(ntree, sizeof(int));
   model->npdf -= 2;
   /* read the number of pdfs */
   if (HTS106_fread_big_endian(&model->npdf[2], sizeof(int), ntree, fp) != ntree)
      result = FALSE;
   for (i = 2; i <= ntree + 1; i++)
      if (model->npdf[i] <= 0) {
         HTS106_error(1, "HTS106_Model_load_pdf: # of pdfs at %d-th state should be positive.\n", i);
         result = FALSE;
         break;
      }
   if (result == FALSE) {
      model->npdf += 2;
      free(model->npdf);
      HTS106_Model_initialize(model);
      return FALSE;
   }
   model->pdf = (double ***) HTS106_calloc(ntree, sizeof(double **));
   model->pdf -= 2;
   /* read means and variances */
   if (msd_flag) {              /* for MSD */
      for (j = 2; j <= ntree + 1; j++) {
         model->pdf[j] = (double **) HTS106_calloc(model->npdf[j], sizeof(double *));
         model->pdf[j]--;
         for (k = 1; k <= model->npdf[j]; k++) {
            model->pdf[j][k] = (double *) HTS106_calloc(2 * model->vector_length + 1, sizeof(double));
            for (l = 0; l < ssize; l++) {
               for (m = 0; m < model->vector_length / ssize; m++) {
                  if (HTS106_fread_big_endian(&temp, sizeof(float), 1, fp) != 1)
                     result = FALSE;
                  model->pdf[j][k][l * model->vector_length / ssize + m] = (double) temp;
                  if (HTS106_fread_big_endian(&temp, sizeof(float), 1, fp) != 1)
                     result = FALSE;
                  model->pdf[j][k][l * model->vector_length / ssize + m + model->vector_length] = (double) temp;
               }
               if (HTS106_fread_big_endian(&temp, sizeof(float), 1, fp) != 1)
                  result = FALSE;
               if (l == 0) {
                  if (temp < 0.0 || temp > 1.0) {
                     HTS106_error(1, "HTS106_Model_load_pdf: MSD weight should be within 0.0 to 1.0.\n");
                     result = FALSE;
                  }
                  model->pdf[j][k][2 * model->vector_length] = (double) temp;
               }
               if (HTS106_fread_big_endian(&temp, sizeof(float), 1, fp) != 1)
                  result = FALSE;
            }
         }
      }
   } else {                     /* for non MSD */
      for (j = 2; j <= ntree + 1; j++) {
         model->pdf[j] = (double **) HTS106_calloc(model->npdf[j], sizeof(double *));
         model->pdf[j]--;
         for (k = 1; k <= model->npdf[j]; k++) {
            model->pdf[j][k] = (double *) HTS106_calloc(2 * model->vector_length, sizeof(double));
            for (l = 0; l < model->vector_length; l++) {
               if (HTS106_fread_big_endian(&temp, sizeof(float), 1, fp) != 1)
                  result = FALSE;
               model->pdf[j][k][l] = (double) temp;
               if (HTS106_fread_big_endian(&temp, sizeof(float), 1, fp) != 1)
                  result = FALSE;
               model->pdf[j][k][l + model->vector_length] = (double) temp;
            }
         }
      }
   }
   if (result == FALSE) {
      HTS106_Model_clear(model);
      return FALSE;
   }
   return TRUE;
}

/* HTS106_Model_load_tree: load trees */
static HTS106_Boolean HTS106_Model_load_tree(HTS106_Model * model, HTS106_File * fp)
{
   char buff[HTS106_MAXBUFLEN];
   HTS106_Question *question, *last_question;
   HTS106_Tree *tree, *last_tree;
   int state;

   /* check */
   if (model == NULL || fp == NULL) {
      HTS106_error(1, "HTS106_Model_load_tree: File for trees is not specified.\n");
      return FALSE;
   }

   model->ntree = 0;
   last_question = NULL;
   last_tree = NULL;
   while (!HTS106_feof(fp)) {
      HTS106_get_pattern_token(fp, buff);
      /* parse questions */
      if (strcmp(buff, "QS") == 0) {
         question = (HTS106_Question *) HTS106_calloc(1, sizeof(HTS106_Question));
         if (HTS106_Question_load(question, fp) == FALSE) {
            free(question);
            HTS106_Model_clear(model);
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
      state = HTS106_get_state_num(buff);
      if (state != 0) {
         tree = (HTS106_Tree *) HTS106_calloc(1, sizeof(HTS106_Tree));
         tree->next = NULL;
         tree->root = NULL;
         tree->head = NULL;
         tree->state = state;
         HTS106_Tree_parse_pattern(tree, buff);
         if (HTS106_Tree_load(tree, fp, model->question) == FALSE) {
            free(tree);
            HTS106_Model_clear(model);
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
   if (model->tree == NULL) {
      HTS106_error(1, "HTS106_Model_load_tree: No trees are loaded.\n");
      return FALSE;
   }
   return TRUE;
}

/* HTS106_Model_clear: free pdfs and trees */
static void HTS106_Model_clear(HTS106_Model * model)
{
   int i, j;
   HTS106_Question *question, *next_question;
   HTS106_Tree *tree, *next_tree;

   for (question = model->question; question; question = next_question) {
      next_question = question->next;
      HTS106_Question_clear(question);
      HTS106_free(question);
   }
   for (tree = model->tree; tree; tree = next_tree) {
      next_tree = tree->next;
      HTS106_Tree_clear(tree);
      HTS106_free(tree);
   }
   if (model->pdf) {
      for (i = 2; i <= model->ntree + 1; i++) {
         for (j = 1; j <= model->npdf[i]; j++) {
            HTS106_free(model->pdf[i][j]);
         }
         model->pdf[i]++;
         HTS106_free(model->pdf[i]);
      }
      model->pdf += 2;
      HTS106_free(model->pdf);
   }
   if (model->npdf) {
      model->npdf += 2;
      HTS106_free(model->npdf);
   }
   HTS106_Model_initialize(model);
}

/* HTS106_Stream_initialize: initialize stream */
static void HTS106_Stream_initialize(HTS106_Stream * stream)
{
   stream->vector_length = 0;
   stream->model = NULL;
   HTS106_Window_initialize(&stream->window);
   stream->msd_flag = FALSE;
   stream->interpolation_size = 0;
}

static void HTS106_Stream_clear(HTS106_Stream * stream);

/* HTS106_Stream_load_pdf: load pdf */
static HTS106_Boolean HTS106_Stream_load_pdf(HTS106_Stream * stream, HTS106_File ** fp, int ntree, HTS106_Boolean msd_flag, int interpolation_size)
{
   int i;
   HTS106_Boolean result = TRUE;

   /* initialize */
   stream->msd_flag = msd_flag;
   stream->interpolation_size = interpolation_size;
   stream->model = (HTS106_Model *) HTS106_calloc(interpolation_size, sizeof(HTS106_Model));
   /* load pdfs */
   for (i = 0; i < stream->interpolation_size; i++) {
      HTS106_Model_initialize(&stream->model[i]);
      if (HTS106_Model_load_pdf(&stream->model[i], fp[i], ntree, stream->msd_flag) == FALSE)
         result = FALSE;
   }
   if (result == FALSE) {
      HTS106_Stream_clear(stream);
      return FALSE;
   }
   /* check */
   for (i = 1; i < stream->interpolation_size; i++) {
      if (stream->model[0].vector_length != stream->model[1].vector_length) {
         HTS106_error(1, "HTS106_Stream_load_pdf: # of states are different in between given modelsets.\n");
         HTS106_Stream_clear(stream);
         return FALSE;
      }
   }
   /* set */
   stream->vector_length = stream->model[0].vector_length;

   return TRUE;
}

/* HTS106_Stream_load_pdf_and_tree: load PDFs and trees */
static HTS106_Boolean HTS106_Stream_load_pdf_and_tree(HTS106_Stream * stream, HTS106_File ** pdf_fp, HTS106_File ** tree_fp, HTS106_Boolean msd_flag, int interpolation_size)
{
   int i;
   HTS106_Boolean result = TRUE;

   /* initialize */
   stream->msd_flag = msd_flag;
   stream->interpolation_size = interpolation_size;
   stream->model = (HTS106_Model *) HTS106_calloc(interpolation_size, sizeof(HTS106_Model));
   /* load */
   for (i = 0; i < stream->interpolation_size; i++) {
      if (!pdf_fp[i]) {
         HTS106_error(1, "HTS106_Stream_load_pdf_and_tree: File for duration PDFs is not specified.\n");
         HTS106_Stream_clear(stream);
         return FALSE;
      }
      if (!tree_fp[i]) {
         HTS106_error(1, "HTS106_Stream_load_pdf_and_tree: File for duration trees is not specified.\n");
         HTS106_Stream_clear(stream);
         return FALSE;
      }
      HTS106_Model_initialize(&stream->model[i]);
      result = HTS106_Model_load_tree(&stream->model[i], tree_fp[i]);
      if (result == FALSE) {
         HTS106_Stream_clear(stream);
         return FALSE;
      }
      result = HTS106_Model_load_pdf(&stream->model[i], pdf_fp[i], stream->model[i].ntree, stream->msd_flag);
      if (result == FALSE) {
         HTS106_Stream_clear(stream);
         return FALSE;
      }
   }
   /* check */
   for (i = 1; i < stream->interpolation_size; i++)
      if (stream->model[0].vector_length != stream->model[i].vector_length) {
         HTS106_error(1, "HTS106_Stream_load_pdf_and_tree: Vector sizes of state output vectors are different in between given modelsets.\n");
         HTS106_Stream_clear(stream);
         return FALSE;
      }
   /* set */
   stream->vector_length = stream->model[0].vector_length;

   return TRUE;
}

/* HTS106_Stream_load_dynamic_window: load windows */
static HTS106_Boolean HTS106_Stream_load_dynamic_window(HTS106_Stream * stream, HTS106_File ** fp, int size)
{
   return HTS106_Window_load(&stream->window, fp, size);
}

/* HTS106_Stream_clear: free stream */
static void HTS106_Stream_clear(HTS106_Stream * stream)
{
   int i;

   if (stream->model) {
      for (i = 0; i < stream->interpolation_size; i++)
         HTS106_Model_clear(&stream->model[i]);
      HTS106_free(stream->model);
   }
   HTS106_Window_clear(&stream->window);
   HTS106_Stream_initialize(stream);
}

/* HTS106_ModelSet_initialize: initialize model set */
void HTS106_ModelSet_initialize(HTS106_ModelSet * ms, int nstream)
{
   HTS106_Stream_initialize(&ms->duration);
   ms->stream = NULL;
   ms->gv = NULL;
   HTS106_Model_initialize(&ms->gv_switch);
   ms->nstate = -1;
   ms->nstream = nstream;
}

/* HTS106_ModelSet_load_duration: load duration model and number of state */
HTS106_Boolean HTS106_ModelSet_load_duration(HTS106_ModelSet * ms, HTS106_File ** pdf_fp, HTS106_File ** tree_fp, int interpolation_size)
{
   /* check */
   if (ms == NULL) {
      return FALSE;
   }
   if (interpolation_size <= 0) {
      HTS106_ModelSet_clear(ms);
      return FALSE;
   }
   if (pdf_fp == NULL) {
      HTS106_error(1, "HTS106_ModelSet_load_duration: File for duration PDFs is not specified.\n");
      HTS106_ModelSet_clear(ms);
      return FALSE;
   }
   if (tree_fp == NULL) {
      HTS106_error(1, "HTS106_ModelSet_load_duration: File for duration trees is not specified.\n");
      HTS106_ModelSet_clear(ms);
      return FALSE;
   }

   if (HTS106_Stream_load_pdf_and_tree(&ms->duration, pdf_fp, tree_fp, FALSE, interpolation_size) == FALSE) {
      HTS106_ModelSet_clear(ms);
      return FALSE;
   }
   ms->nstate = ms->duration.vector_length;

   return TRUE;
}

/* HTS106_ModelSet_load_parameter: load model */
HTS106_Boolean HTS106_ModelSet_load_parameter(HTS106_ModelSet * ms, HTS106_File ** pdf_fp, HTS106_File ** tree_fp, HTS106_File ** win_fp, int stream_index, HTS106_Boolean msd_flag, int window_size, int interpolation_size)
{
   int i;

   /* check */
   if (ms == NULL) {
      return FALSE;
   }
   if (stream_index < 0 || stream_index >= ms->nstream || window_size <= 0 || interpolation_size <= 0) {
      HTS106_ModelSet_clear(ms);
      return FALSE;
   }
   if (pdf_fp == NULL) {
      HTS106_error(1, "HTS106_ModelSet_load_parameter: File for pdfs is not specified.\n");
      HTS106_ModelSet_clear(ms);
      return FALSE;
   }
   if (tree_fp == NULL) {
      HTS106_error(1, "HTS106_ModelSet_load_parameter: File for wins is not specified.\n");
      HTS106_ModelSet_clear(ms);
      return FALSE;
   }
   if (win_fp == NULL) {
      HTS106_error(1, "HTS106_ModelSet_load_parameter: File for wins is not specified.\n");
      HTS106_ModelSet_clear(ms);
      return FALSE;
   }
   /* initialize */
   if (!ms->stream) {
      ms->stream = (HTS106_Stream *) HTS106_calloc(ms->nstream, sizeof(HTS106_Stream));
      for (i = 0; i < ms->nstream; i++)
         HTS106_Stream_initialize(&ms->stream[i]);
   }
   /* load */
   if (HTS106_Stream_load_pdf_and_tree(&ms->stream[stream_index], pdf_fp, tree_fp, msd_flag, interpolation_size) == FALSE) {
      HTS106_ModelSet_clear(ms);
      return FALSE;
   }
   if (HTS106_Stream_load_dynamic_window(&ms->stream[stream_index], win_fp, window_size) == FALSE) {
      HTS106_ModelSet_clear(ms);
      return FALSE;
   }

   return TRUE;
}

/* HTS106_ModelSet_load_gv: load GV model */
HTS106_Boolean HTS106_ModelSet_load_gv(HTS106_ModelSet * ms, HTS106_File ** pdf_fp, HTS106_File ** tree_fp, int stream_index, int interpolation_size)
{
   int i;

   /* check */
   if (ms == NULL) {
      return FALSE;
   }
   if (stream_index < 0 || stream_index >= ms->nstream || interpolation_size <= 0) {
      HTS106_ModelSet_clear(ms);
      return FALSE;
   }
   if (pdf_fp == NULL) {
      HTS106_error(1, "HTS106_ModelSet_load_gv: File for GV pdfs is not specified.\n");
      HTS106_ModelSet_clear(ms);
      return FALSE;
   }
   /* initialize */
   if (!ms->gv) {
      ms->gv = (HTS106_Stream *) HTS106_calloc(ms->nstream, sizeof(HTS106_Stream));
      for (i = 0; i < ms->nstream; i++)
         HTS106_Stream_initialize(&ms->gv[i]);
   }
   if (tree_fp) {
      if (HTS106_Stream_load_pdf_and_tree(&ms->gv[stream_index], pdf_fp, tree_fp, FALSE, interpolation_size) == FALSE) {
         HTS106_ModelSet_clear(ms);
         return FALSE;
      }
   } else {
      if (HTS106_Stream_load_pdf(&ms->gv[stream_index], pdf_fp, 1, FALSE, interpolation_size) == FALSE) {
         HTS106_ModelSet_clear(ms);
         return FALSE;
      }
   }

   return TRUE;
}

/* HTS106_ModelSet_have_gv_tree: if context-dependent GV is used, return true */
HTS106_Boolean HTS106_ModelSet_have_gv_tree(HTS106_ModelSet * ms, int stream_index)
{
   int i;

   for (i = 0; i < ms->gv[stream_index].interpolation_size; i++)
      if (ms->gv[stream_index].model[i].tree == NULL)
         return FALSE;
   return TRUE;
}

/* HTS106_ModelSet_load_gv_switch: load GV switch */
HTS106_Boolean HTS106_ModelSet_load_gv_switch(HTS106_ModelSet * ms, HTS106_File * fp)
{
   if (fp != NULL)
      return HTS106_Model_load_tree(&ms->gv_switch, fp);
   return FALSE;
}

/* HTS106_ModelSet_have_gv_switch: if GV switch is used, return true */
HTS106_Boolean HTS106_ModelSet_have_gv_switch(HTS106_ModelSet * ms)
{
   if (ms->gv_switch.tree != NULL)
      return TRUE;
   else
      return FALSE;
}

/* HTS106_ModelSet_get_nstate: get number of state */
int HTS106_ModelSet_get_nstate(HTS106_ModelSet * ms)
{
   return ms->nstate;
}

/* HTS106_ModelSet_get_nstream: get number of stream */
int HTS106_ModelSet_get_nstream(HTS106_ModelSet * ms)
{
   return ms->nstream;
}

/* HTS106_ModelSet_get_vector_length: get vector length */
int HTS106_ModelSet_get_vector_length(HTS106_ModelSet * ms, int stream_index)
{
   return ms->stream[stream_index].vector_length;
}

/* HTS106_ModelSet_is_msd: get MSD flag */
HTS106_Boolean HTS106_ModelSet_is_msd(HTS106_ModelSet * ms, int stream_index)
{
   return ms->stream[stream_index].msd_flag;
}

/* HTS106_ModelSet_get_window_size: get dynamic window size */
int HTS106_ModelSet_get_window_size(HTS106_ModelSet * ms, int stream_index)
{
   return ms->stream[stream_index].window.size;
}

/* HTS106_ModelSet_get_window_left_width: get left width of dynamic window */
int HTS106_ModelSet_get_window_left_width(HTS106_ModelSet * ms, int stream_index, int window_index)
{
   return ms->stream[stream_index].window.l_width[window_index];
}

/* HTS106_ModelSet_get_window_right_width: get right width of dynamic window */
int HTS106_ModelSet_get_window_right_width(HTS106_ModelSet * ms, int stream_index, int window_index)
{
   return ms->stream[stream_index].window.r_width[window_index];
}

/* HTS106_ModelSet_get_window_coefficient: get coefficient of dynamic window */
double HTS106_ModelSet_get_window_coefficient(HTS106_ModelSet * ms, int stream_index, int window_index, int coefficient_index)
{
   return ms->stream[stream_index].window.coefficient[window_index][coefficient_index];
}

/* HTS106_ModelSet_get_window_max_width: get max width of dynamic window */
int HTS106_ModelSet_get_window_max_width(HTS106_ModelSet * ms, int stream_index)
{
   return ms->stream[stream_index].window.max_width;
}

/* HTS106_ModelSet_get_duration_interpolation_size: get interpolation size (duration model) */
int HTS106_ModelSet_get_duration_interpolation_size(HTS106_ModelSet * ms)
{
   return ms->duration.interpolation_size;
}

/* HTS106_ModelSet_get_parameter_interpolation_size: get interpolation size (parameter model) */
int HTS106_ModelSet_get_parameter_interpolation_size(HTS106_ModelSet * ms, int stream_index)
{
   return ms->stream[stream_index].interpolation_size;
}

/* HTS106_ModelSet_get_gv_interpolation_size: get interpolation size (GV model) */
int HTS106_ModelSet_get_gv_interpolation_size(HTS106_ModelSet * ms, int stream_index)
{
   return ms->gv[stream_index].interpolation_size;
}

/* HTS106_ModelSet_use_gv: get GV flag */
HTS106_Boolean HTS106_ModelSet_use_gv(HTS106_ModelSet * ms, int stream_index)
{
   if (!ms->gv)
      return FALSE;
   if (ms->gv[stream_index].vector_length > 0)
      return TRUE;
   return FALSE;
}

/* HTS106_ModelSet_get_duration_index: get index of duration tree and PDF */
void HTS106_ModelSet_get_duration_index(HTS106_ModelSet * ms, char *string, const RHVoice_parsed_label_string* parsed, int *tree_index, int *pdf_index, int interpolation_index)
{
   HTS106_Tree *tree;
   HTS106_Pattern *pattern;
   HTS106_Boolean find;

   find = FALSE;
   (*tree_index) = 2;
   (*pdf_index) = 1;
   for (tree = ms->duration.model[interpolation_index].tree; tree; tree = tree->next) {
      pattern = tree->head;
      if (!pattern)
         find = TRUE;
      for (; pattern; pattern = pattern->next)
         if (HTS106_pattern_match(string, pattern->string)) {
            find = TRUE;
            break;
         }
      if (find)
         break;
      (*tree_index)++;
   }

   if (tree == NULL) {
      HTS106_error(1, "HTS106_ModelSet_get_duration_index: Cannot find model %s.\n", string);
      return;
   }
   (*pdf_index) = HTS106_Tree_search_node(tree, string, parsed);
}

/* HTS106_ModelSet_get_duration: get duration using interpolation weight */
void HTS106_ModelSet_get_duration(HTS106_ModelSet * ms, char *string, const RHVoice_parsed_label_string* parsed, double *mean, double *vari, double *iw)
{
   int i, j;
   int tree_index, pdf_index;
   const int vector_length = ms->duration.vector_length;

   for (i = 0; i < ms->nstate; i++) {
      mean[i] = 0.0;
      vari[i] = 0.0;
   }
   for (i = 0; i < ms->duration.interpolation_size; i++) {
     HTS106_ModelSet_get_duration_index(ms, string, parsed, &tree_index, &pdf_index, i);
      for (j = 0; j < ms->nstate; j++) {
         mean[j] += iw[i] * ms->duration.model[i].pdf[tree_index][pdf_index][j];
         vari[j] += iw[i] * iw[i] * ms->duration.model[i].pdf[tree_index][pdf_index][j + vector_length];
      }
   }
}

/* HTS106_ModelSet_get_parameter_index: get index of parameter tree and PDF */
void HTS106_ModelSet_get_parameter_index(HTS106_ModelSet * ms, char *string, const RHVoice_parsed_label_string* parsed, int *tree_index, int *pdf_index, int stream_index, int state_index, int interpolation_index)
{
   HTS106_Tree *tree;
   HTS106_Pattern *pattern;
   HTS106_Boolean find;

   find = FALSE;
   (*tree_index) = 2;
   (*pdf_index) = 1;
   for (tree = ms->stream[stream_index].model[interpolation_index].tree; tree; tree = tree->next) {
      if (tree->state == state_index) {
         pattern = tree->head;
         if (!pattern)
            find = TRUE;
         for (; pattern; pattern = pattern->next)
            if (HTS106_pattern_match(string, pattern->string)) {
               find = TRUE;
               break;
            }
         if (find)
            break;
      }
      (*tree_index)++;
   }

   if (tree == NULL) {
      HTS106_error(1, "HTS106_ModelSet_get_parameter_index: Cannot find model %s.\n", string);
      return;
   }
   (*pdf_index) = HTS106_Tree_search_node(tree, string, parsed);
}

/* HTS106_ModelSet_get_parameter: get parameter using interpolation weight */
void HTS106_ModelSet_get_parameter(HTS106_ModelSet * ms, char *string, const RHVoice_parsed_label_string* parsed, double *mean, double *vari, double *msd, int stream_index, int state_index, double *iw)
{
   int i, j;
   int tree_index, pdf_index;
   const int vector_length = ms->stream[stream_index].vector_length;

   for (i = 0; i < vector_length; i++) {
      mean[i] = 0.0;
      vari[i] = 0.0;
   }
   if (msd)
      *msd = 0.0;
   for (i = 0; i < ms->stream[stream_index].interpolation_size; i++) {
     HTS106_ModelSet_get_parameter_index(ms, string, parsed, &tree_index, &pdf_index, stream_index, state_index, i);
      for (j = 0; j < vector_length; j++) {
         mean[j] += iw[i] * ms->stream[stream_index].model[i].pdf[tree_index][pdf_index][j];
         vari[j] += iw[i] * iw[i] * ms->stream[stream_index].model[i]
             .pdf[tree_index][pdf_index][j + vector_length];
      }
      if (ms->stream[stream_index].msd_flag) {
         *msd += iw[i] * ms->stream[stream_index].model[i]
             .pdf[tree_index][pdf_index][2 * vector_length];
      }
   }
}

/* HTS106_ModelSet_get_gv_index: get index of GV tree and PDF */
void HTS106_ModelSet_get_gv_index(HTS106_ModelSet * ms, char *string, int *tree_index, int *pdf_index, int stream_index, int interpolation_index)
{
   HTS106_Tree *tree;
   HTS106_Pattern *pattern;
   HTS106_Boolean find;

   find = FALSE;
   (*tree_index) = 2;
   (*pdf_index) = 1;

   if (HTS106_ModelSet_have_gv_tree(ms, stream_index) == FALSE)
      return;
   for (tree = ms->gv[stream_index].model[interpolation_index].tree; tree; tree = tree->next) {
      pattern = tree->head;
      if (!pattern)
         find = TRUE;
      for (; pattern; pattern = pattern->next)
         if (HTS106_pattern_match(string, pattern->string)) {
            find = TRUE;
            break;
         }
      if (find)
         break;
      (*tree_index)++;
   }

   if (tree == NULL) {
      HTS106_error(1, "HTS106_ModelSet_get_gv_index: Cannot find model %s.\n", string);
      return;
   }
   (*pdf_index) = HTS106_Tree_search_node(tree, string, NULL);
}

/* HTS106_ModelSet_get_gv: get GV using interpolation weight */
void HTS106_ModelSet_get_gv(HTS106_ModelSet * ms, char *string, double *mean, double *vari, int stream_index, double *iw)
{
   int i, j;
   int tree_index, pdf_index;
   const int vector_length = ms->gv[stream_index].vector_length;

   for (i = 0; i < vector_length; i++) {
      mean[i] = 0.0;
      vari[i] = 0.0;
   }
   for (i = 0; i < ms->gv[stream_index].interpolation_size; i++) {
      HTS106_ModelSet_get_gv_index(ms, string, &tree_index, &pdf_index, stream_index, i);
      for (j = 0; j < vector_length; j++) {
         mean[j] += iw[i] * ms->gv[stream_index].model[i].pdf[tree_index][pdf_index][j];
         vari[j] += iw[i] * iw[i] * ms->gv[stream_index].model[i]
             .pdf[tree_index][pdf_index][j + vector_length];
      }
   }
}

/* HTS106_ModelSet_get_gv_switch_index: get index of GV switch tree and PDF */
void HTS106_ModelSet_get_gv_switch_index(HTS106_ModelSet * ms, char *string, int *tree_index, int *pdf_index)
{
   HTS106_Tree *tree;
   HTS106_Pattern *pattern;
   HTS106_Boolean find;

   find = FALSE;
   (*tree_index) = 2;
   (*pdf_index) = 1;
   for (tree = ms->gv_switch.tree; tree; tree = tree->next) {
      pattern = tree->head;
      if (!pattern)
         find = TRUE;
      for (; pattern; pattern = pattern->next)
         if (HTS106_pattern_match(string, pattern->string)) {
            find = TRUE;
            break;
         }
      if (find)
         break;
      (*tree_index)++;
   }

   if (tree == NULL) {
      HTS106_error(1, "HTS106_ModelSet_get_gv_switch_index: Cannot find model %s.\n", string);
      return;
   }
   (*pdf_index) = HTS106_Tree_search_node(tree, string, NULL);
}

/* HTS106_ModelSet_get_gv_switch: get GV switch */
HTS106_Boolean HTS106_ModelSet_get_gv_switch(HTS106_ModelSet * ms, char *string)
{
   int tree_index, pdf_index;

   if (ms->gv_switch.tree == NULL)
      return TRUE;
   HTS106_ModelSet_get_gv_switch_index(ms, string, &tree_index, &pdf_index);
   if (pdf_index == 1)
      return FALSE;
   else
      return TRUE;
}

/* HTS106_ModelSet_clear: free model set */
void HTS106_ModelSet_clear(HTS106_ModelSet * ms)
{
   int i;

   HTS106_Stream_clear(&ms->duration);
   if (ms->stream) {
      for (i = 0; i < ms->nstream; i++)
         HTS106_Stream_clear(&ms->stream[i]);
      HTS106_free(ms->stream);
   }
   if (ms->gv) {
      for (i = 0; i < ms->nstream; i++)
         HTS106_Stream_clear(&ms->gv[i]);
      HTS106_free(ms->gv);
   }
   HTS106_Model_clear(&ms->gv_switch);
   HTS106_ModelSet_initialize(ms, -1);
}

HTS106_MODEL_C_END;

#endif                          /* !HTS106_MODEL_C */
