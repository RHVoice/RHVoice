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

/* HTS_dp_match: recursive matching */
static HTS_Boolean HTS_dp_match(const char *string, const char *pattern, const int pos, const int max)
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
   int i, j;
   int buff_length, max = 0, nstar = 0, nquestion = 0;
   char buff[HTS_MAXBUFLEN];
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
      return HTS_dp_match(string, pattern, 0, (int) (strlen(string) - max));
}

/* HTS_is_num: check given buffer is number or not */
static HTS_Boolean HTS_is_num(const char *buff)
{
   int i;
   const int length = (int) strlen(buff);

   for (i = 0; i < length; i++)
      if (!(isdigit((int) buff[i]) || (buff[i] == '-')))
         return FALSE;

   return TRUE;
}

/* HTS_name2num: convert name of node to number */
static int HTS_name2num(const char *buff)
{
   int i;

   for (i = strlen(buff) - 1; '0' <= buff[i] && buff[i] <= '9' && i >= 0; i--);
   i++;

   return atoi(&buff[i]);
}

/* HTS_get_state_num: return the number of state */
static int HTS_get_state_num(char *string)
{
   char *left, *right;

   if (((left = strchr(string, '[')) == NULL)
       || ((right = strrchr(string, ']')) == NULL))
      return 0;
   *right = '\0';
   string = left + 1;

   return atoi(string);
}

static void HTS_Question_clear(HTS_Question * question);

/* HTS_Question_load: Load questions from file */
static HTS_Boolean HTS_Question_load(HTS_Question * question, HTS_File * fp)
{
   char buff[HTS_MAXBUFLEN];
   HTS_Pattern *pattern, *last_pattern;

   if (question == NULL || fp == NULL)
      return FALSE;

   /* get question name */
   if (HTS_get_pattern_token(fp, buff) == FALSE)
      return FALSE;
   question->string = HTS_strdup(buff);
   question->head = NULL;
   /* get pattern list */
   if (HTS_get_pattern_token(fp, buff) == FALSE) {
      free(question->string);
      question->string = NULL;
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
         if (question->head)
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
static HTS_Boolean HTS_Question_match(const HTS_Question * question, const char *string)
{
   HTS_Pattern *pattern;

   for (pattern = question->head; pattern; pattern = pattern->next)
      if (HTS_pattern_match(string, pattern->string))
         return TRUE;

   return FALSE;
}

/* HTS_Question_find_question: find question from question list */
static HTS_Question *HTS_Question_find_question(HTS_Question * question, const char *buff)
{

   for (; question; question = question->next)
      if (strcmp(buff, question->string) == 0)
         return question;

   HTS_error(1, "HTS_Question_find_question: Cannot find question %s.\n", buff);
   return NULL;                 /* make compiler happy */
}

/* HTS_Question_clear: clear loaded question */
static void HTS_Question_clear(HTS_Question * question)
{
   HTS_Pattern *pattern, *next_pattern;

   HTS_free(question->string);
   for (pattern = question->head; pattern; pattern = next_pattern) {
      next_pattern = pattern->next;
      HTS_free(pattern->string);
      HTS_free(pattern);
   }
}

/* HTS_Node_find: find node for given number */
static HTS_Node *HTS_Node_find(HTS_Node * node, const int num)
{
   for (; node; node = node->next)
      if (node->index == num)
         return node;

   HTS_error(1, "HTS_Node_find: Cannot find node %d.\n", num);
   return NULL;                 /* make compiler happy */
}

/* HTS_Node_clear: recursive function to free Node */
static void HTS_Node_clear(HTS_Node * node)
{
   if (node->yes != NULL)
      HTS_Node_clear(node->yes);
   if (node->no != NULL)
      HTS_Node_clear(node->no);
   HTS_free(node);
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

static void HTS_Tree_clear(HTS_Tree * tree);

/* HTS_Tree_load: Load trees */
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
   node->index = 0;
   tree->root = last_node = node;

   if (strcmp(buff, "{") == 0) {
      while (HTS_get_pattern_token(fp, buff) == TRUE && strcmp(buff, "}") != 0) {
         node = HTS_Node_find(last_node, atoi(buff));
         if (node == NULL) {
            HTS_Tree_clear(tree);
            return FALSE;
         }
         if (HTS_get_pattern_token(fp, buff) == FALSE) {
            HTS_Tree_clear(tree);
            return FALSE;
         }
         node->quest = HTS_Question_find_question(question, buff);
         if (node->quest == NULL) {
            HTS_Tree_clear(tree);
            return FALSE;
         }
         node->yes = (HTS_Node *) HTS_calloc(1, sizeof(HTS_Node));
         node->no = (HTS_Node *) HTS_calloc(1, sizeof(HTS_Node));

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
static int HTS_Tree_search_node(HTS_Tree * tree, const char *string)
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

   HTS_error(1, "HTS_Tree_search_node: Cannot find node.\n");
   return -1;                   /* make compiler happy */
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

   HTS_Node_clear(tree->root);
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

static void HTS_Window_clear(HTS_Window * win);

/* HTS_Window_load: load dynamic windows */
static HTS_Boolean HTS_Window_load(HTS_Window * win, HTS_File ** fp, int size)
{
   int i, j;
   int fsize, length;
   char buff[HTS_MAXBUFLEN];
   HTS_Boolean result = TRUE;

   /* check */
   if (win == NULL || fp == NULL || size <= 0)
      return FALSE;

   win->size = size;
   win->l_width = (int *) HTS_calloc(win->size, sizeof(int));
   win->r_width = (int *) HTS_calloc(win->size, sizeof(int));
   win->coefficient = (double **) HTS_calloc(win->size, sizeof(double *));
   /* set delta coefficents */
   for (i = 0; i < win->size; i++) {
      if (HTS_get_token(fp[i], buff) == FALSE) {
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
      win->coefficient[i] = (double *) HTS_calloc(fsize, sizeof(double));
      for (j = 0; j < fsize; j++) {
         if (HTS_get_token(fp[i], buff) == FALSE) {
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
      HTS_Window_clear(win);
      return FALSE;
   }
   return TRUE;
}

/* HTS_Window_clear: free dynamic window */
static void HTS_Window_clear(HTS_Window * win)
{
   int i;

   if (win->coefficient) {
      for (i = win->size - 1; i >= 0; i--) {
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

/* HTS_Model_initialize: initialize model */
static void HTS_Model_initialize(HTS_Model * model)
{
   model->vector_length = 0;
   model->ntree = 0;
   model->npdf = NULL;
   model->pdf = NULL;
   model->tree = NULL;
   model->question = NULL;
}

static void HTS_Model_clear(HTS_Model * model);

/* HTS_Model_load_pdf: load pdfs */
static HTS_Boolean HTS_Model_load_pdf(HTS_Model * model, HTS_File * fp, int ntree, HTS_Boolean msd_flag)
{
   int i, j, k, l, m;
   float temp;
   int ssize;
   HTS_Boolean result = TRUE;

   /* check */
   if (model == NULL || fp == NULL || ntree <= 0) {
      HTS_error(1, "HTS_Model_load_pdf: File for pdfs is not specified.\n");
      return FALSE;
   }

   /* load pdf */
   model->ntree = ntree;
   /* read MSD flag */
   HTS_fread_big_endian(&i, sizeof(int), 1, fp);
   if ((i != 0 || msd_flag != FALSE) && (i != 1 || msd_flag != TRUE)) {
      HTS_error(1, "HTS_Model_load_pdf: Failed to load header of pdfs.\n");
      HTS_Model_initialize(model);
      return FALSE;
   }
   /* read stream size */
   HTS_fread_big_endian(&ssize, sizeof(int), 1, fp);
   if (ssize < 1) {
      HTS_error(1, "HTS_Model_load_pdf: Failed to load header of pdfs.\n");
      HTS_Model_initialize(model);
      return FALSE;
   }
   /* read vector size */
   HTS_fread_big_endian(&model->vector_length, sizeof(int), 1, fp);
   if (model->vector_length <= 0) {
      HTS_error(1, "HTS_Model_load_pdf: # of HMM states %d should be positive.\n", model->vector_length);
      HTS_Model_initialize(model);
      return FALSE;
   }
   model->npdf = (int *) HTS_calloc(ntree, sizeof(int));
   model->npdf -= 2;
   /* read the number of pdfs */
   if (HTS_fread_big_endian(&model->npdf[2], sizeof(int), ntree, fp) != ntree)
      result = FALSE;
   for (i = 2; i <= ntree + 1; i++)
      if (model->npdf[i] <= 0) {
         HTS_error(1, "HTS_Model_load_pdf: # of pdfs at %d-th state should be positive.\n", i);
         result = FALSE;
         break;
      }
   if (result == FALSE) {
      model->npdf += 2;
      free(model->npdf);
      HTS_Model_initialize(model);
      return FALSE;
   }
   model->pdf = (double ***) HTS_calloc(ntree, sizeof(double **));
   model->pdf -= 2;
   /* read means and variances */
   if (msd_flag) {              /* for MSD */
      for (j = 2; j <= ntree + 1; j++) {
         model->pdf[j] = (double **) HTS_calloc(model->npdf[j], sizeof(double *));
         model->pdf[j]--;
         for (k = 1; k <= model->npdf[j]; k++) {
            model->pdf[j][k] = (double *) HTS_calloc(2 * model->vector_length + 1, sizeof(double));
            for (l = 0; l < ssize; l++) {
               for (m = 0; m < model->vector_length / ssize; m++) {
                  if (HTS_fread_big_endian(&temp, sizeof(float), 1, fp) != 1)
                     result = FALSE;
                  model->pdf[j][k][l * model->vector_length / ssize + m] = (double) temp;
                  if (HTS_fread_big_endian(&temp, sizeof(float), 1, fp) != 1)
                     result = FALSE;
                  model->pdf[j][k][l * model->vector_length / ssize + m + model->vector_length] = (double) temp;
               }
               if (HTS_fread_big_endian(&temp, sizeof(float), 1, fp) != 1)
                  result = FALSE;
               if (l == 0) {
                  if (temp < 0.0 || temp > 1.0) {
                     HTS_error(1, "HTS_Model_load_pdf: MSD weight should be within 0.0 to 1.0.\n");
                     result = FALSE;
                  }
                  model->pdf[j][k][2 * model->vector_length] = (double) temp;
               }
               if (HTS_fread_big_endian(&temp, sizeof(float), 1, fp) != 1)
                  result = FALSE;
            }
         }
      }
   } else {                     /* for non MSD */
      for (j = 2; j <= ntree + 1; j++) {
         model->pdf[j] = (double **) HTS_calloc(model->npdf[j], sizeof(double *));
         model->pdf[j]--;
         for (k = 1; k <= model->npdf[j]; k++) {
            model->pdf[j][k] = (double *) HTS_calloc(2 * model->vector_length, sizeof(double));
            for (l = 0; l < model->vector_length; l++) {
               if (HTS_fread_big_endian(&temp, sizeof(float), 1, fp) != 1)
                  result = FALSE;
               model->pdf[j][k][l] = (double) temp;
               if (HTS_fread_big_endian(&temp, sizeof(float), 1, fp) != 1)
                  result = FALSE;
               model->pdf[j][k][l + model->vector_length] = (double) temp;
            }
         }
      }
   }
   if (result == FALSE) {
      HTS_Model_clear(model);
      return FALSE;
   }
   return TRUE;
}

/* HTS_Model_load_tree: load trees */
static HTS_Boolean HTS_Model_load_tree(HTS_Model * model, HTS_File * fp)
{
   char buff[HTS_MAXBUFLEN];
   HTS_Question *question, *last_question;
   HTS_Tree *tree, *last_tree;
   int state;

   /* check */
   if (model == NULL || fp == NULL) {
      HTS_error(1, "HTS_Model_load_tree: File for trees is not specified.\n");
      return FALSE;
   }

   model->ntree = 0;
   last_question = NULL;
   last_tree = NULL;
   while (!HTS_feof(fp)) {
      HTS_get_pattern_token(fp, buff);
      /* parse questions */
      if (strcmp(buff, "QS") == 0) {
         question = (HTS_Question *) HTS_calloc(1, sizeof(HTS_Question));
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
         tree->next = NULL;
         tree->root = NULL;
         tree->head = NULL;
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
   if (model->tree == NULL) {
      HTS_error(1, "HTS_Model_load_tree: No trees are loaded.\n");
      return FALSE;
   }
   return TRUE;
}

/* HTS_Model_clear: free pdfs and trees */
static void HTS_Model_clear(HTS_Model * model)
{
   int i, j;
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

/* HTS_Stream_initialize: initialize stream */
static void HTS_Stream_initialize(HTS_Stream * stream)
{
   stream->vector_length = 0;
   stream->model = NULL;
   HTS_Window_initialize(&stream->window);
   stream->msd_flag = FALSE;
   stream->interpolation_size = 0;
}

static void HTS_Stream_clear(HTS_Stream * stream);

/* HTS_Stream_load_pdf: load pdf */
static HTS_Boolean HTS_Stream_load_pdf(HTS_Stream * stream, HTS_File ** fp, int ntree, HTS_Boolean msd_flag, int interpolation_size)
{
   int i;
   HTS_Boolean result = TRUE;

   /* initialize */
   stream->msd_flag = msd_flag;
   stream->interpolation_size = interpolation_size;
   stream->model = (HTS_Model *) HTS_calloc(interpolation_size, sizeof(HTS_Model));
   /* load pdfs */
   for (i = 0; i < stream->interpolation_size; i++) {
      HTS_Model_initialize(&stream->model[i]);
      if (HTS_Model_load_pdf(&stream->model[i], fp[i], ntree, stream->msd_flag) == FALSE)
         result = FALSE;
   }
   if (result == FALSE) {
      HTS_Stream_clear(stream);
      return FALSE;
   }
   /* check */
   for (i = 1; i < stream->interpolation_size; i++) {
      if (stream->model[0].vector_length != stream->model[1].vector_length) {
         HTS_error(1, "HTS_Stream_load_pdf: # of states are different in between given modelsets.\n");
         HTS_Stream_clear(stream);
         return FALSE;
      }
   }
   /* set */
   stream->vector_length = stream->model[0].vector_length;

   return TRUE;
}

/* HTS_Stream_load_pdf_and_tree: load PDFs and trees */
static HTS_Boolean HTS_Stream_load_pdf_and_tree(HTS_Stream * stream, HTS_File ** pdf_fp, HTS_File ** tree_fp, HTS_Boolean msd_flag, int interpolation_size)
{
   int i;
   HTS_Boolean result = TRUE;

   /* initialize */
   stream->msd_flag = msd_flag;
   stream->interpolation_size = interpolation_size;
   stream->model = (HTS_Model *) HTS_calloc(interpolation_size, sizeof(HTS_Model));
   /* load */
   for (i = 0; i < stream->interpolation_size; i++) {
      if (!pdf_fp[i]) {
         HTS_error(1, "HTS_Stream_load_pdf_and_tree: File for duration PDFs is not specified.\n");
         HTS_Stream_clear(stream);
         return FALSE;
      }
      if (!tree_fp[i]) {
         HTS_error(1, "HTS_Stream_load_pdf_and_tree: File for duration trees is not specified.\n");
         HTS_Stream_clear(stream);
         return FALSE;
      }
      HTS_Model_initialize(&stream->model[i]);
      result = HTS_Model_load_tree(&stream->model[i], tree_fp[i]);
      if (result == FALSE) {
         HTS_Stream_clear(stream);
         return FALSE;
      }
      result = HTS_Model_load_pdf(&stream->model[i], pdf_fp[i], stream->model[i].ntree, stream->msd_flag);
      if (result == FALSE) {
         HTS_Stream_clear(stream);
         return FALSE;
      }
   }
   /* check */
   for (i = 1; i < stream->interpolation_size; i++)
      if (stream->model[0].vector_length != stream->model[i].vector_length) {
         HTS_error(1, "HTS_Stream_load_pdf_and_tree: Vector sizes of state output vectors are different in between given modelsets.\n");
         HTS_Stream_clear(stream);
         return FALSE;
      }
   /* set */
   stream->vector_length = stream->model[0].vector_length;

   return TRUE;
}

/* HTS_Stream_load_dynamic_window: load windows */
static HTS_Boolean HTS_Stream_load_dynamic_window(HTS_Stream * stream, HTS_File ** fp, int size)
{
   return HTS_Window_load(&stream->window, fp, size);
}

/* HTS_Stream_clear: free stream */
static void HTS_Stream_clear(HTS_Stream * stream)
{
   int i;

   if (stream->model) {
      for (i = 0; i < stream->interpolation_size; i++)
         HTS_Model_clear(&stream->model[i]);
      HTS_free(stream->model);
   }
   HTS_Window_clear(&stream->window);
   HTS_Stream_initialize(stream);
}

/* HTS_ModelSet_initialize: initialize model set */
void HTS_ModelSet_initialize(HTS_ModelSet * ms, int nstream)
{
   HTS_Stream_initialize(&ms->duration);
   ms->stream = NULL;
   ms->gv = NULL;
   HTS_Model_initialize(&ms->gv_switch);
   ms->nstate = -1;
   ms->nstream = nstream;
}

/* HTS_ModelSet_load_duration: load duration model and number of state */
HTS_Boolean HTS_ModelSet_load_duration(HTS_ModelSet * ms, HTS_File ** pdf_fp, HTS_File ** tree_fp, int interpolation_size)
{
   /* check */
   if (ms == NULL) {
      return FALSE;
   }
   if (interpolation_size <= 0) {
      HTS_ModelSet_clear(ms);
      return FALSE;
   }
   if (pdf_fp == NULL) {
      HTS_error(1, "HTS_ModelSet_load_duration: File for duration PDFs is not specified.\n");
      HTS_ModelSet_clear(ms);
      return FALSE;
   }
   if (tree_fp == NULL) {
      HTS_error(1, "HTS_ModelSet_load_duration: File for duration trees is not specified.\n");
      HTS_ModelSet_clear(ms);
      return FALSE;
   }

   if (HTS_Stream_load_pdf_and_tree(&ms->duration, pdf_fp, tree_fp, FALSE, interpolation_size) == FALSE) {
      HTS_ModelSet_clear(ms);
      return FALSE;
   }
   ms->nstate = ms->duration.vector_length;

   return TRUE;
}

/* HTS_ModelSet_load_parameter: load model */
HTS_Boolean HTS_ModelSet_load_parameter(HTS_ModelSet * ms, HTS_File ** pdf_fp, HTS_File ** tree_fp, HTS_File ** win_fp, int stream_index, HTS_Boolean msd_flag, int window_size, int interpolation_size)
{
   int i;

   /* check */
   if (ms == NULL) {
      return FALSE;
   }
   if (stream_index < 0 || stream_index >= ms->nstream || window_size <= 0 || interpolation_size <= 0) {
      HTS_ModelSet_clear(ms);
      return FALSE;
   }
   if (pdf_fp == NULL) {
      HTS_error(1, "HTS_ModelSet_load_parameter: File for pdfs is not specified.\n");
      HTS_ModelSet_clear(ms);
      return FALSE;
   }
   if (tree_fp == NULL) {
      HTS_error(1, "HTS_ModelSet_load_parameter: File for wins is not specified.\n");
      HTS_ModelSet_clear(ms);
      return FALSE;
   }
   if (win_fp == NULL) {
      HTS_error(1, "HTS_ModelSet_load_parameter: File for wins is not specified.\n");
      HTS_ModelSet_clear(ms);
      return FALSE;
   }
   /* initialize */
   if (!ms->stream) {
      ms->stream = (HTS_Stream *) HTS_calloc(ms->nstream, sizeof(HTS_Stream));
      for (i = 0; i < ms->nstream; i++)
         HTS_Stream_initialize(&ms->stream[i]);
   }
   /* load */
   if (HTS_Stream_load_pdf_and_tree(&ms->stream[stream_index], pdf_fp, tree_fp, msd_flag, interpolation_size) == FALSE) {
      HTS_ModelSet_clear(ms);
      return FALSE;
   }
   if (HTS_Stream_load_dynamic_window(&ms->stream[stream_index], win_fp, window_size) == FALSE) {
      HTS_ModelSet_clear(ms);
      return FALSE;
   }

   return TRUE;
}

/* HTS_ModelSet_load_gv: load GV model */
HTS_Boolean HTS_ModelSet_load_gv(HTS_ModelSet * ms, HTS_File ** pdf_fp, HTS_File ** tree_fp, int stream_index, int interpolation_size)
{
   int i;

   /* check */
   if (ms == NULL) {
      return FALSE;
   }
   if (stream_index < 0 || stream_index >= ms->nstream || interpolation_size <= 0) {
      HTS_ModelSet_clear(ms);
      return FALSE;
   }
   if (pdf_fp == NULL) {
      HTS_error(1, "HTS_ModelSet_load_gv: File for GV pdfs is not specified.\n");
      HTS_ModelSet_clear(ms);
      return FALSE;
   }
   /* initialize */
   if (!ms->gv) {
      ms->gv = (HTS_Stream *) HTS_calloc(ms->nstream, sizeof(HTS_Stream));
      for (i = 0; i < ms->nstream; i++)
         HTS_Stream_initialize(&ms->gv[i]);
   }
   if (tree_fp) {
      if (HTS_Stream_load_pdf_and_tree(&ms->gv[stream_index], pdf_fp, tree_fp, FALSE, interpolation_size) == FALSE) {
         HTS_ModelSet_clear(ms);
         return FALSE;
      }
   } else {
      if (HTS_Stream_load_pdf(&ms->gv[stream_index], pdf_fp, 1, FALSE, interpolation_size) == FALSE) {
         HTS_ModelSet_clear(ms);
         return FALSE;
      }
   }

   return TRUE;
}

/* HTS_ModelSet_have_gv_tree: if context-dependent GV is used, return true */
HTS_Boolean HTS_ModelSet_have_gv_tree(HTS_ModelSet * ms, int stream_index)
{
   int i;

   for (i = 0; i < ms->gv[stream_index].interpolation_size; i++)
      if (ms->gv[stream_index].model[i].tree == NULL)
         return FALSE;
   return TRUE;
}

/* HTS_ModelSet_load_gv_switch: load GV switch */
HTS_Boolean HTS_ModelSet_load_gv_switch(HTS_ModelSet * ms, HTS_File * fp)
{
   if (fp != NULL)
      return HTS_Model_load_tree(&ms->gv_switch, fp);
   return FALSE;
}

/* HTS_ModelSet_have_gv_switch: if GV switch is used, return true */
HTS_Boolean HTS_ModelSet_have_gv_switch(HTS_ModelSet * ms)
{
   if (ms->gv_switch.tree != NULL)
      return TRUE;
   else
      return FALSE;
}

/* HTS_ModelSet_get_nstate: get number of state */
int HTS_ModelSet_get_nstate(HTS_ModelSet * ms)
{
   return ms->nstate;
}

/* HTS_ModelSet_get_nstream: get number of stream */
int HTS_ModelSet_get_nstream(HTS_ModelSet * ms)
{
   return ms->nstream;
}

/* HTS_ModelSet_get_vector_length: get vector length */
int HTS_ModelSet_get_vector_length(HTS_ModelSet * ms, int stream_index)
{
   return ms->stream[stream_index].vector_length;
}

/* HTS_ModelSet_is_msd: get MSD flag */
HTS_Boolean HTS_ModelSet_is_msd(HTS_ModelSet * ms, int stream_index)
{
   return ms->stream[stream_index].msd_flag;
}

/* HTS_ModelSet_get_window_size: get dynamic window size */
int HTS_ModelSet_get_window_size(HTS_ModelSet * ms, int stream_index)
{
   return ms->stream[stream_index].window.size;
}

/* HTS_ModelSet_get_window_left_width: get left width of dynamic window */
int HTS_ModelSet_get_window_left_width(HTS_ModelSet * ms, int stream_index, int window_index)
{
   return ms->stream[stream_index].window.l_width[window_index];
}

/* HTS_ModelSet_get_window_right_width: get right width of dynamic window */
int HTS_ModelSet_get_window_right_width(HTS_ModelSet * ms, int stream_index, int window_index)
{
   return ms->stream[stream_index].window.r_width[window_index];
}

/* HTS_ModelSet_get_window_coefficient: get coefficient of dynamic window */
double HTS_ModelSet_get_window_coefficient(HTS_ModelSet * ms, int stream_index, int window_index, int coefficient_index)
{
   return ms->stream[stream_index].window.coefficient[window_index][coefficient_index];
}

/* HTS_ModelSet_get_window_max_width: get max width of dynamic window */
int HTS_ModelSet_get_window_max_width(HTS_ModelSet * ms, int stream_index)
{
   return ms->stream[stream_index].window.max_width;
}

/* HTS_ModelSet_get_duration_interpolation_size: get interpolation size (duration model) */
int HTS_ModelSet_get_duration_interpolation_size(HTS_ModelSet * ms)
{
   return ms->duration.interpolation_size;
}

/* HTS_ModelSet_get_parameter_interpolation_size: get interpolation size (parameter model) */
int HTS_ModelSet_get_parameter_interpolation_size(HTS_ModelSet * ms, int stream_index)
{
   return ms->stream[stream_index].interpolation_size;
}

/* HTS_ModelSet_get_gv_interpolation_size: get interpolation size (GV model) */
int HTS_ModelSet_get_gv_interpolation_size(HTS_ModelSet * ms, int stream_index)
{
   return ms->gv[stream_index].interpolation_size;
}

/* HTS_ModelSet_use_gv: get GV flag */
HTS_Boolean HTS_ModelSet_use_gv(HTS_ModelSet * ms, int stream_index)
{
   if (!ms->gv)
      return FALSE;
   if (ms->gv[stream_index].vector_length > 0)
      return TRUE;
   return FALSE;
}

/* HTS_ModelSet_get_duration_index: get index of duration tree and PDF */
void HTS_ModelSet_get_duration_index(HTS_ModelSet * ms, char *string, int *tree_index, int *pdf_index, int interpolation_index)
{
   HTS_Tree *tree;
   HTS_Pattern *pattern;
   HTS_Boolean find;

   find = FALSE;
   (*tree_index) = 2;
   (*pdf_index) = 1;
   for (tree = ms->duration.model[interpolation_index].tree; tree; tree = tree->next) {
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
      (*tree_index)++;
   }

   if (tree == NULL) {
      HTS_error(1, "HTS_ModelSet_get_duration_index: Cannot find model %s.\n", string);
      return;
   }
   (*pdf_index) = HTS_Tree_search_node(tree, string);
}

/* HTS_ModelSet_get_duration: get duration using interpolation weight */
void HTS_ModelSet_get_duration(HTS_ModelSet * ms, char *string, double *mean, double *vari, double *iw)
{
   int i, j;
   int tree_index, pdf_index;
   const int vector_length = ms->duration.vector_length;

   for (i = 0; i < ms->nstate; i++) {
      mean[i] = 0.0;
      vari[i] = 0.0;
   }
   for (i = 0; i < ms->duration.interpolation_size; i++) {
      HTS_ModelSet_get_duration_index(ms, string, &tree_index, &pdf_index, i);
      for (j = 0; j < ms->nstate; j++) {
         mean[j] += iw[i] * ms->duration.model[i].pdf[tree_index][pdf_index][j];
         vari[j] += iw[i] * iw[i] * ms->duration.model[i].pdf[tree_index][pdf_index][j + vector_length];
      }
   }
}

/* HTS_ModelSet_get_parameter_index: get index of parameter tree and PDF */
void HTS_ModelSet_get_parameter_index(HTS_ModelSet * ms, char *string, int *tree_index, int *pdf_index, int stream_index, int state_index, int interpolation_index)
{
   HTS_Tree *tree;
   HTS_Pattern *pattern;
   HTS_Boolean find;

   find = FALSE;
   (*tree_index) = 2;
   (*pdf_index) = 1;
   for (tree = ms->stream[stream_index].model[interpolation_index].tree; tree; tree = tree->next) {
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

   if (tree == NULL) {
      HTS_error(1, "HTS_ModelSet_get_parameter_index: Cannot find model %s.\n", string);
      return;
   }
   (*pdf_index) = HTS_Tree_search_node(tree, string);
}

/* HTS_ModelSet_get_parameter: get parameter using interpolation weight */
void HTS_ModelSet_get_parameter(HTS_ModelSet * ms, char *string, double *mean, double *vari, double *msd, int stream_index, int state_index, double *iw)
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
      HTS_ModelSet_get_parameter_index(ms, string, &tree_index, &pdf_index, stream_index, state_index, i);
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

/* HTS_ModelSet_get_gv_index: get index of GV tree and PDF */
void HTS_ModelSet_get_gv_index(HTS_ModelSet * ms, char *string, int *tree_index, int *pdf_index, int stream_index, int interpolation_index)
{
   HTS_Tree *tree;
   HTS_Pattern *pattern;
   HTS_Boolean find;

   find = FALSE;
   (*tree_index) = 2;
   (*pdf_index) = 1;

   if (HTS_ModelSet_have_gv_tree(ms, stream_index) == FALSE)
      return;
   for (tree = ms->gv[stream_index].model[interpolation_index].tree; tree; tree = tree->next) {
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
      (*tree_index)++;
   }

   if (tree == NULL) {
      HTS_error(1, "HTS_ModelSet_get_gv_index: Cannot find model %s.\n", string);
      return;
   }
   (*pdf_index) = HTS_Tree_search_node(tree, string);
}

/* HTS_ModelSet_get_gv: get GV using interpolation weight */
void HTS_ModelSet_get_gv(HTS_ModelSet * ms, char *string, double *mean, double *vari, int stream_index, double *iw)
{
   int i, j;
   int tree_index, pdf_index;
   const int vector_length = ms->gv[stream_index].vector_length;

   for (i = 0; i < vector_length; i++) {
      mean[i] = 0.0;
      vari[i] = 0.0;
   }
   for (i = 0; i < ms->gv[stream_index].interpolation_size; i++) {
      HTS_ModelSet_get_gv_index(ms, string, &tree_index, &pdf_index, stream_index, i);
      for (j = 0; j < vector_length; j++) {
         mean[j] += iw[i] * ms->gv[stream_index].model[i].pdf[tree_index][pdf_index][j];
         vari[j] += iw[i] * iw[i] * ms->gv[stream_index].model[i]
             .pdf[tree_index][pdf_index][j + vector_length];
      }
   }
}

/* HTS_ModelSet_get_gv_switch_index: get index of GV switch tree and PDF */
void HTS_ModelSet_get_gv_switch_index(HTS_ModelSet * ms, char *string, int *tree_index, int *pdf_index)
{
   HTS_Tree *tree;
   HTS_Pattern *pattern;
   HTS_Boolean find;

   find = FALSE;
   (*tree_index) = 2;
   (*pdf_index) = 1;
   for (tree = ms->gv_switch.tree; tree; tree = tree->next) {
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
      (*tree_index)++;
   }

   if (tree == NULL) {
      HTS_error(1, "HTS_ModelSet_get_gv_switch_index: Cannot find model %s.\n", string);
      return;
   }
   (*pdf_index) = HTS_Tree_search_node(tree, string);
}

/* HTS_ModelSet_get_gv_switch: get GV switch */
HTS_Boolean HTS_ModelSet_get_gv_switch(HTS_ModelSet * ms, char *string)
{
   int tree_index, pdf_index;

   if (ms->gv_switch.tree == NULL)
      return TRUE;
   HTS_ModelSet_get_gv_switch_index(ms, string, &tree_index, &pdf_index);
   if (pdf_index == 1)
      return FALSE;
   else
      return TRUE;
}

/* HTS_ModelSet_clear: free model set */
void HTS_ModelSet_clear(HTS_ModelSet * ms)
{
   int i;

   HTS_Stream_clear(&ms->duration);
   if (ms->stream) {
      for (i = 0; i < ms->nstream; i++)
         HTS_Stream_clear(&ms->stream[i]);
      HTS_free(ms->stream);
   }
   if (ms->gv) {
      for (i = 0; i < ms->nstream; i++)
         HTS_Stream_clear(&ms->gv[i]);
      HTS_free(ms->gv);
   }
   HTS_Model_clear(&ms->gv_switch);
   HTS_ModelSet_initialize(ms, -1);
}

HTS_MODEL_C_END;

#endif                          /* !HTS_MODEL_C */
