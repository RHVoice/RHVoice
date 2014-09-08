/* ----------------------------------------------------------------- */
/*             The Speech Signal Processing Toolkit (SPTK)           */
/*             developed by SPTK Working Group                       */
/*             http://sp-tk.sourceforge.net/                         */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 1984-2007  Tokyo Institute of Technology           */
/*                           Interdisciplinary Graduate School of    */
/*                           Science and Engineering                 */
/*                                                                   */
/*                1996-2008  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
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
/* - Neither the name of the SPTK working group nor the names of its */
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

/********************************************************************
    $Id: getmem.c,v 1.10 2008/06/16 05:47:54 heigazen Exp $

    Memory Allocation Functions

    short  *sgetmem(leng)
    long   *lgetmem(leng)
    double *dgetmem(leng)
    float  *fgetmem(leng)
    real   *rgetmem(leng)
    float  **fgetmem(leng)

    int leng : data length

    char *getmem(leng, size)

    size_t leng : data length
    size_t size : size of data type

**********************************************************************/

#include "getmem-sptk.h"

char *getmem (const size_t leng, const size_t size)
{
   char *p = NULL;

   if ((p = (char *)calloc(leng, size)) == NULL){
      fprintf(stderr, "Cannot allocate memory!\n");
      exit(3);
   }
   return (p);
}

short *sgetmem (const int leng)
{
   return ( (short *)getmem((size_t)leng, sizeof(short)) );
}

long *lgetmem (const int leng)
{
   return ( (long *)getmem((size_t)leng, sizeof(long)) );
}

double *dgetmem (const int leng)
{
   return ( (double *)getmem((size_t)leng, sizeof(double)) );
}

float *fgetmem (const int leng)
{
   return ( (float *)getmem((size_t)leng, sizeof(float)) );
}

real *rgetmem (const int leng)
{
   return ( (real *)getmem((size_t)leng, sizeof(real)) );
}

float **ffgetmem (const int leng)
{
   return ( (float **)getmem((size_t)leng, sizeof(float *)) );
}
