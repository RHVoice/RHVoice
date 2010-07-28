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

#include <stdlib.h>             /* for exit(),calloc(),free() */
#include <stdarg.h>             /* for va_list */
#include <string.h>             /* for strcpy(),strlen() */

/* hts_engine libraries */
#include "HTS_hidden.h"

#ifdef FESTIVAL
#include "EST_walloc.h"
#endif                          /* FESTIVAL */

/* HTS_byte_swap: byte swap */
static int HTS_byte_swap(void *p, const int size, const int block)
{
   char *q, tmp;
   int i, j;

   q = (char *) p;

   for (i = 0; i < block; i++) {
      for (j = 0; j < (size / 2); j++) {
         tmp = *(q + j);
         *(q + j) = *(q + (size - 1 - j));
         *(q + (size - 1 - j)) = tmp;
      }
      q += size;
   }

   return i;
}

/* HTS_error: output error message */
void HTS_error(const int error, char *message, ...)
{
   va_list arg;

   fflush(stdout);
   fflush(stderr);

   if (error > 0)
      fprintf(stderr, "\nError: ");
   else
      fprintf(stderr, "\nWarning: ");

   va_start(arg, message);
   vfprintf(stderr, message, arg);
   va_end(arg);

   fflush(stderr);

   if (error > 0)
      exit(error);
}

/* HTS_get_fp: wrapper for fopen */
FILE *HTS_get_fp(const char *name, const char *opt)
{
   FILE *fp = fopen(name, opt);

   if (fp == NULL)
      HTS_error(2, "HTS_get_fp: Cannot open %s.\n", name);

   return (fp);
}

/* HTS_get_pattern_token: get pattern token */
void HTS_get_pattern_token(FILE * fp, char *buff)
{
   char c;
   int i;
   HTS_Boolean squote = FALSE, dquote = FALSE;

   c = fgetc(fp);

   while (c == ' ' || c == '\n')
      c = fgetc(fp);

   if (c == '\'') {             /* single quote case */
      c = fgetc(fp);
      squote = TRUE;
   }

   if (c == '\"') {             /*double quote case */
      c = fgetc(fp);
      dquote = TRUE;
   }

   if (c == ',') {              /*special character ',' */
      strcpy(buff, ",");
      return;
   }

   i = 0;
   while (1) {
      buff[i++] = c;
      c = fgetc(fp);
      if (squote && c == '\'')
         break;
      if (dquote && c == '\"')
         break;
      if (!squote && !dquote) {
         if (c == ' ')
            break;
         if (c == '\n')
            break;
         if (feof(fp))
            break;
      }
   }

   buff[i] = '\0';
}

/* HTS_get_token: get token (separator are space,tab,line break) */
HTS_Boolean HTS_get_token(FILE * fp, char *buff)
{
   char c;
   int i;

   if (feof(fp))
      return FALSE;
   c = fgetc(fp);
   while (c == ' ' || c == '\n' || c == '\t') {
      if (feof(fp))
         return FALSE;
      c = getc(fp);
   }

   for (i = 0; c != ' ' && c != '\n' && c != '\t' && !feof(fp); i++) {
      buff[i] = c;
      c = fgetc(fp);
   }

   buff[i] = '\0';
   return TRUE;
}

/* HTS_get_token_from_string: get token from string (separator are space,tab,line break) */
HTS_Boolean HTS_get_token_from_string(char *string, int *index, char *buff)
{
   char c;
   int i;

   c = string[(*index)];
   if (c == '\0')
      return FALSE;
   c = string[(*index)++];
   if (c == '\0')
      return FALSE;
   while (c == ' ' || c == '\n' || c == '\t') {
      if (c == '\0')
         return FALSE;
      c = string[(*index)++];
   }
   for (i = 0; c != ' ' && c != '\n' && c != '\t' && c != '\0'; i++) {
      buff[i] = c;
      c = string[(*index)++];
   }

   buff[i] = '\0';
   return TRUE;
}

/* HTS_fread_big_endian: fread with byteswap */
int HTS_fread_big_endian(void *p, const int size, const int num, FILE * fp)
{
   const int block = fread(p, size, num, fp);

#ifdef WORDS_LITTLEENDIAN
   HTS_byte_swap(p, size, block);
#endif                          /* WORDS_LITTLEENDIAN */

   return block;
}

/* HTS_fwrite_little_endian: fwrite with byteswap */
int HTS_fwrite_little_endian(void *p, const int size, const int num, FILE * fp)
{
   const int block = num * size;

#ifdef WORDS_BIGENDIAN
   HTS_byte_swap(p, size, block);
#endif                          /* WORDS_BIGENDIAN */
   fwrite(p, size, num, fp);

   return block;
}

/* HTS_calloc: wrapper for calloc */
char *HTS_calloc(const size_t num, const size_t size)
{
#ifdef FESTIVAL
   char *mem = (char *) safe_wcalloc(num * size);
#else
   char *mem = (char *) calloc(num, size);
#endif                          /* FESTIVAL */

   if (mem == NULL)
      HTS_error(1, "HTS_calloc: Cannot allocate memory.\n");

   return mem;
}

/* HTS_Free: wrapper for free */
void HTS_free(void *ptr)
{
#ifdef FESTIVAL
   wfree(ptr);
#else
   free(ptr);
#endif                          /* FESTIVAL */
}

/* HTS_strdup: wrapper for strdup */
char *HTS_strdup(const char *string)
{
#ifdef FESTIVAL
   return (wstrdup(string));
#else
   char *buff = (char *) HTS_calloc(strlen(string) + 1, sizeof(char));
   strcpy(buff, string);
   return buff;
#endif                          /* FESTIVAL */
}

/* HTS_alloc_matrix: allocate double matrix */
double **HTS_alloc_matrix(const int x, const int y)
{
   int i;
   double **p = (double **) HTS_calloc(x, sizeof(double *));

   for (i = 0; i < x; i++)
      p[i] = (double *) HTS_calloc(y, sizeof(double));
   return p;
}

/* HTS_free_matrix: free double matrix */
void HTS_free_matrix(double **p, const int x)
{
   int i;

   for (i = x - 1; i >= 0; i--)
      HTS_free(p[i]);
   HTS_free(p);
}
