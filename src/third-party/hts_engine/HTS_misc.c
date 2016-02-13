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

#ifndef HTS_MISC_C
#define HTS_MISC_C

#ifdef __cplusplus
#define HTS_MISC_C_START extern "C" {
#define HTS_MISC_C_END   }
#else
#define HTS_MISC_C_START
#define HTS_MISC_C_END
#endif                          /* __CPLUSPLUS */

HTS_MISC_C_START;

#include <stdlib.h>             /* for exit(),calloc(),free() */
#include <stdarg.h>             /* for va_list */
#include <string.h>             /* for strcpy(),strlen() */

/* hts_engine libraries */
#include "HTS_hidden.h"

#ifdef FESTIVAL
#include "EST_walloc.h"
#endif                          /* FESTIVAL */

#define HTS_FILE  0
#define HTS_DATA  1

typedef struct _HTS_Data {
   unsigned char *data;
   size_t size;
   size_t index;
} HTS_Data;

/* HTS_fopen_from_fn: wrapper for fopen */
HTS_File *HTS_fopen_from_fn(const char *name, const char *opt)
{
   HTS_File *fp = (HTS_File *) HTS_calloc(1, sizeof(HTS_File));

   fp->type = HTS_FILE;
   fp->pointer = (void *) fopen(name, opt);

   if (fp->pointer == NULL) {
      HTS_error(0, "HTS_fopen: Cannot open %s.\n", name);
      HTS_free(fp);
      return NULL;
   }

   return fp;
}

/* HTS_fopen_from_fp: wrapper for fopen */
HTS_File *HTS_fopen_from_fp(HTS_File * fp, size_t size)
{
   if (fp == NULL || size == 0)
      return NULL;
   else if (fp->type == HTS_FILE) {
      HTS_Data *d;
      HTS_File *f;
      d = (HTS_Data *) HTS_calloc(1, sizeof(HTS_Data));
      d->data = (unsigned char *) HTS_calloc(size, sizeof(unsigned char));
      d->size = size;
      d->index = 0;
      if (fread(d->data, sizeof(unsigned char), size, (FILE *) fp->pointer) != size) {
         free(d->data);
         free(d);
         return NULL;
      }
      f = (HTS_File *) HTS_calloc(1, sizeof(HTS_File));
      f->type = HTS_DATA;
      f->pointer = (void *) d;
      return f;
   } else if (fp->type == HTS_DATA) {
      HTS_File *f;
      HTS_Data *tmp1, *tmp2;
      tmp1 = (HTS_Data *) fp->pointer;
      if (tmp1->index + size > tmp1->size)
         return NULL;
      tmp2 = (HTS_Data *) HTS_calloc(1, sizeof(HTS_Data));
      tmp2->data = (unsigned char *) HTS_calloc(size, sizeof(unsigned char));
      tmp2->size = size;
      tmp2->index = 0;
      memcpy(tmp2->data, &tmp1->data[tmp1->index], size);
      tmp1->index += size;
      f = (HTS_File *) HTS_calloc(1, sizeof(HTS_File));
      f->type = HTS_DATA;
      f->pointer = (void *) tmp2;
      return f;
   }

   HTS_error(0, "HTS_fopen_from_fp: Unknown file type.\n");
   return NULL;
}

/* HTS_fopen_from_data: wrapper for fopen */
HTS_File *HTS_fopen_from_data(void *data, size_t size)
{
   HTS_Data *d;
   HTS_File *f;

   if (data == NULL || size == 0)
      return NULL;

   d = (HTS_Data *) HTS_calloc(1, sizeof(HTS_Data));
   d->data = (unsigned char *) HTS_calloc(size, sizeof(unsigned char));
   d->size = size;
   d->index = 0;

   memcpy(d->data, data, size);

   f = (HTS_File *) HTS_calloc(1, sizeof(HTS_File));
   f->type = HTS_DATA;
   f->pointer = (void *) d;

   return f;
}

/* HTS_fclose: wrapper for fclose */
void HTS_fclose(HTS_File * fp)
{
   if (fp == NULL) {
      return;
   } else if (fp->type == HTS_FILE) {
      if (fp->pointer != NULL)
         fclose((FILE *) fp->pointer);
      HTS_free(fp);
      return;
   } else if (fp->type == HTS_DATA) {
      if (fp->pointer != NULL) {
         HTS_Data *d = (HTS_Data *) fp->pointer;
         if (d->data != NULL)
            HTS_free(d->data);
         HTS_free(d);
      }
      HTS_free(fp);
      return;
   }
   HTS_error(0, "HTS_fclose: Unknown file type.\n");
}

/* HTS_fgetc: wrapper for fgetc */
int HTS_fgetc(HTS_File * fp)
{
   if (fp == NULL) {
      return EOF;
   } else if (fp->type == HTS_FILE) {
      return fgetc((FILE *) fp->pointer);
   } else if (fp->type == HTS_DATA) {
      HTS_Data *d = (HTS_Data *) fp->pointer;
      if (d->size <= d->index)
         return EOF;
      return (int) d->data[d->index++];
   }
   HTS_error(0, "HTS_fgetc: Unknown file type.\n");
   return EOF;
}

/* HTS_feof: wrapper for feof */
int HTS_feof(HTS_File * fp)
{
   if (fp == NULL) {
      return 1;
   } else if (fp->type == HTS_FILE) {
      return feof((FILE *) fp->pointer);
   } else if (fp->type == HTS_DATA) {
      HTS_Data *d = (HTS_Data *) fp->pointer;
      return d->size <= d->index ? 1 : 0;
   }
   HTS_error(0, "HTS_feof: Unknown file type.\n");
   return 1;
}

/* HTS_fseek: wrapper for fseek */
int HTS_fseek(HTS_File * fp, long offset, int origin)
{
   if (fp == NULL) {
      return 1;
   } else if (fp->type == HTS_FILE) {
      return fseek((FILE *) fp->pointer, offset, origin);
   } else if (fp->type == HTS_DATA) {
      HTS_Data *d = (HTS_Data *) fp->pointer;
      if (origin == SEEK_SET) {
         d->index = (size_t) offset;
      } else if (origin == SEEK_CUR) {
         d->index += offset;
      } else if (origin == SEEK_END) {
         d->index = d->size + offset;
      } else {
         return 1;
      }
      return 0;
   }
   HTS_error(0, "HTS_fseek: Unknown file type.\n");
   return 1;
}

/* HTS_ftell: rapper for ftell */
size_t HTS_ftell(HTS_File * fp)
{
   if (fp == NULL) {
      return 0;
   } else if (fp->type == HTS_FILE) {
      fpos_t pos;
      fgetpos((FILE *) fp->pointer, &pos);
#if defined(_WIN32) || defined(__CYGWIN__) || defined(__APPLE__) || defined(__ANDROID__)
      return (size_t) pos;
#else
      return (size_t) pos.__pos;
#endif                          /* _WIN32 || __CYGWIN__ || __APPLE__ || __ANDROID__ */
   } else if (fp->type == HTS_DATA) {
      HTS_Data *d = (HTS_Data *) fp->pointer;
      return d->index;
   }
   HTS_error(0, "HTS_ftell: Unknown file type.\n");
   return 0;
}

/* HTS_fread: wrapper for fread */
static size_t HTS_fread(void *buf, size_t size, size_t n, HTS_File * fp)
{
   if (fp == NULL || size == 0 || n == 0) {
      return 0;
   }
   if (fp->type == HTS_FILE) {
      return fread(buf, size, n, (FILE *) fp->pointer);
   } else if (fp->type == HTS_DATA) {
      HTS_Data *d = (HTS_Data *) fp->pointer;
      size_t i, length = size * n;
      unsigned char *c = (unsigned char *) buf;
      for (i = 0; i < length; i++) {
         if (d->index < d->size)
            c[i] = d->data[d->index++];
         else
            break;
      }
      if (i == 0)
         return 0;
      else
         return i / size;
   }
   HTS_error(0, "HTS_fread: Unknown file type.\n");
   return 0;
}

/* HTS_byte_swap: byte swap */
static void HTS_byte_swap(void *p, size_t size, size_t block)
{
   char *q, tmp;
   size_t i, j;

   q = (char *) p;

   for (i = 0; i < block; i++) {
      for (j = 0; j < (size / 2); j++) {
         tmp = *(q + j);
         *(q + j) = *(q + (size - 1 - j));
         *(q + (size - 1 - j)) = tmp;
      }
      q += size;
   }
}

/* HTS_fread_big_endian: fread with byteswap */
size_t HTS_fread_big_endian(void *buf, size_t size, size_t n, HTS_File * fp)
{
   size_t block = HTS_fread(buf, size, n, fp);

#ifdef WORDS_LITTLEENDIAN
   HTS_byte_swap(buf, size, block);
#endif                          /* WORDS_LITTLEENDIAN */

   return block;
}

/* HTS_fread_little_endian: fread with byteswap */
size_t HTS_fread_little_endian(void *buf, size_t size, size_t n, HTS_File * fp)
{
   size_t block = HTS_fread(buf, size, n, fp);

#ifdef WORDS_BIGENDIAN
   HTS_byte_swap(buf, size, block);
#endif                          /* WORDS_BIGENDIAN */

   return block;
}

/* HTS_fwrite_little_endian: fwrite with byteswap */
size_t HTS_fwrite_little_endian(const void *buf, size_t size, size_t n, FILE * fp)
{
#ifdef WORDS_BIGENDIAN
   HTS_byte_swap(buf, size, n * size);
#endif                          /* WORDS_BIGENDIAN */
   return fwrite(buf, size, n, fp);
}

/* HTS_get_pattern_token: get pattern token (single/double quote can be used) */
HTS_Boolean HTS_get_pattern_token(HTS_File * fp, char *buff)
{
   char c;
   size_t i;
   HTS_Boolean squote = FALSE, dquote = FALSE;

   if (fp == NULL || HTS_feof(fp))
      return FALSE;
   c = HTS_fgetc(fp);

   while (c == ' ' || c == '\n') {
      if (HTS_feof(fp))
         return FALSE;
      c = HTS_fgetc(fp);
   }

   if (c == '\'') {             /* single quote case */
      if (HTS_feof(fp))
         return FALSE;
      c = HTS_fgetc(fp);
      squote = TRUE;
   }

   if (c == '\"') {             /*double quote case */
      if (HTS_feof(fp))
         return FALSE;
      c = HTS_fgetc(fp);
      dquote = TRUE;
   }

   if (c == ',') {              /*special character ',' */
      strcpy(buff, ",");
      return TRUE;
   }

   i = 0;
   while (1) {
      buff[i++] = c;
      c = HTS_fgetc(fp);
      if (squote && c == '\'')
         break;
      if (dquote && c == '\"')
         break;
      if (!squote && !dquote) {
         if (c == ' ')
            break;
         if (c == '\n')
            break;
         if (HTS_feof(fp))
            break;
      }
   }

   buff[i] = '\0';
   return TRUE;
}

/* HTS_get_token: get token from file pointer (separators are space, tab, and line break) */
HTS_Boolean HTS_get_token_from_fp(HTS_File * fp, char *buff)
{
   char c;
   size_t i;

   if (fp == NULL || HTS_feof(fp))
      return FALSE;
   c = HTS_fgetc(fp);
   while (c == ' ' || c == '\n' || c == '\t') {
      if (HTS_feof(fp))
         return FALSE;
      c = HTS_fgetc(fp);
      if (c == EOF)
         return FALSE;
   }

   for (i = 0; c != ' ' && c != '\n' && c != '\t';) {
      buff[i++] = c;
      if (HTS_feof(fp))
         break;
      c = HTS_fgetc(fp);
      if (c == EOF)
         break;
   }

   buff[i] = '\0';
   return TRUE;
}

/* HTS_get_token_with_separator: get token from file pointer with specified separator */
HTS_Boolean HTS_get_token_from_fp_with_separator(HTS_File * fp, char *buff, char separator)
{
   char c;
   size_t i;

   if (fp == NULL || HTS_feof(fp))
      return FALSE;
   c = HTS_fgetc(fp);
   while (c == separator) {
      if (HTS_feof(fp))
         return FALSE;
      c = HTS_fgetc(fp);
      if (c == EOF)
         return FALSE;
   }

   for (i = 0; c != separator;) {
      buff[i++] = c;
      if (HTS_feof(fp))
         break;
      c = HTS_fgetc(fp);
      if (c == EOF)
         break;
   }

   buff[i] = '\0';
   return TRUE;
}

/* HTS_get_token_from_string: get token from string (separators are space, tab, and line break) */
HTS_Boolean HTS_get_token_from_string(const char *string, size_t * index, char *buff)
{
   char c;
   size_t i;

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

/* HTS_get_token_from_string_with_separator: get token from string with specified separator */
HTS_Boolean HTS_get_token_from_string_with_separator(const char *str, size_t * index, char *buff, char separator)
{
   char c;
   size_t len = 0;

   if (str == NULL)
      return FALSE;

   c = str[(*index)];
   if (c == '\0')
      return FALSE;
   while (c == separator) {
      if (c == '\0')
         return FALSE;
      (*index)++;
      c = str[(*index)];
   }
   while (c != separator && c != '\0') {
      buff[len++] = c;
      (*index)++;
      c = str[(*index)];
   }
   if (c != '\0')
      (*index)++;

   buff[len] = '\0';

   if (len > 0)
      return TRUE;
   else
      return FALSE;
}

/* HTS_calloc: wrapper for calloc */
void *HTS_calloc(const size_t num, const size_t size)
{
   size_t n = num * size;
   void *mem;

   if (n == 0)
      return NULL;

#ifdef FESTIVAL
   mem = (void *) safe_wcalloc(n);
#else
   mem = (void *) malloc(n);
#endif                          /* FESTIVAL */

   memset(mem, 0, n);

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
double **HTS_alloc_matrix(size_t x, size_t y)
{
   size_t i;
   double **p;

   if (x == 0 || y == 0)
      return NULL;

   p = (double **) HTS_calloc(x, sizeof(double *));

   for (i = 0; i < x; i++)
      p[i] = (double *) HTS_calloc(y, sizeof(double));
   return p;
}

/* HTS_free_matrix: free double matrix */
void HTS_free_matrix(double **p, size_t x)
{
   size_t i;

   for (i = 0; i < x; i++)
      HTS_free(p[i]);
   HTS_free(p);
}

/* HTS_error: output error message */
void HTS_error(int error, const char *message, ...)
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

HTS_MISC_C_END;

#endif                          /* !HTS_MISC_C */
