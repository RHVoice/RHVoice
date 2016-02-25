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

/********************************************************
   $Id: _fft.c,v 1.11 2008/06/16 05:48:36 heigazen Exp $
       NAME:
                fft - fast fourier transform
       SYNOPSIS:
                int   fft(x, y, m);

                double   x[];   real part
                double   y[];   imaginary part
                int      m;     data size

                return : success = 0
                         fault   = -1
       Naohiro Isshiki          Dec.1995    modified
********************************************************/

#include "fft-sptk.h"

double *_sintbl=0;
int maxfftsize=0;

static int checkm (const int m)
{
   int k;

   for (k=4; k<=m; k<<=1) {
      if (k==m)
         return(0);
   }
   fprintf(stderr, "fft : m must be a integer of power of 2!\n");

   return(-1);
}

int fft (double *x, double *y, const int m)
{
   int j, lmx, li;
   double *xp, *yp;
   double *sinp,*cosp;
   int lf, lix,tblsize;
   int mv2, mm1;
   double t1, t2;
   double  arg;
   int checkm(const int);

   /**************
   * RADIX-2 FFT *
   **************/

   if (checkm(m))
      return(-1);

   /***********************
   * SIN table generation *
   ***********************/

   if ((_sintbl==0) || (maxfftsize<m)) {
      tblsize=m-m/4+1;
      arg=M_PI/m*2;
      if (_sintbl!=0)
         free(_sintbl);
      _sintbl = sinp = dgetmem(tblsize);
      *sinp++ = 0;
      for (j=1; j<tblsize; j++)
         *sinp++ = sin(arg*(double)j);
      _sintbl[m/2] = 0;
      maxfftsize = m;
   }

   lf = maxfftsize / m;
   lmx = m;

   for(;;) {
      lix = lmx;
      lmx /= 2;
      if (lmx <= 1) break;
      sinp = _sintbl;
      cosp = _sintbl + maxfftsize/4;
      for (j=0; j<lmx; j++) {
         xp = &x[j];
         yp = &y[j];
         for (li=lix; li<=m ; li+=lix) {
            t1 = *(xp) - *(xp + lmx);
            t2 = *(yp) - *(yp + lmx);
            *(xp) += *(xp + lmx);
            *(yp) += *(yp + lmx);
            *(xp + lmx) = *cosp * t1 + *sinp * t2;
            *(yp + lmx) = *cosp * t2 - *sinp * t1;
            xp += lix;
            yp += lix;
         }
         sinp += lf;
         cosp += lf;
      }
      lf += lf;
   }

   xp = x;
   yp = y;
   for (li=m/2; li--; xp+=2,yp+=2) {
      t1 = *(xp) - *(xp + 1);
      t2 = *(yp) - *(yp + 1);
      *(xp) += *(xp + 1);
      *(yp) += *(yp + 1);
      *(xp + 1) = t1;
      *(yp + 1) = t2;
   }

   /***************
   * bit reversal *
   ***************/
   j = 0;
   xp = x;
   yp = y;
   mv2 = m / 2;
   mm1 = m - 1;
   for (lmx=0; lmx<mm1; lmx++) {
      if ((li=lmx-j)<0) {
         t1 = *(xp);
         t2 = *(yp);
         *(xp) = *(xp + li);
         *(yp) = *(yp + li);
         *(xp + li) = t1;
         *(yp + li) = t2;
      }
      li = mv2;
      while (li<=j) {
         j -= li;
         li /= 2;
      }
      j += li;
      xp = x + j;
      yp = y + j;
   }

   return(0);
}

/*****************************************************************
*  $Id: _ifft.c,v 1.14 2008/06/16 05:48:33 heigazen Exp $         *
*  NAME:                                                         *
*      ifft - Inverse Fast Fourier Transform                     *
*  SYNOPSIS:                                                     *
*      int   ifft(x, y, m)                                       *
*                                                                *
*      real   x[];   real part                                   *
*      real   y[];   imaginary part                              *
*      int    m;     size of FFT                                 *
*****************************************************************/

//from file _ifft.c of SPTK 3.2
int ifft (double *x, double *y, const int m)
{
   int i;

   if (fft(y, x, m)==-1)
      return(-1);

   for (i=m; --i>=0; ++x, ++y) {
      *x /= m;
      *y /= m;
   }

   return(0);
}

/********************************************************
 $Id: _fftr.c,v 1.11 2008/06/16 05:48:33 heigazen Exp $

 NAME:
        fftr - Fast Fourier Transform for Double sequence
 SYNOPSIS:
        int   fftr(x, y, m)

        double  x[];   real part of data
        double  y[];   working area
        int     m;     number of data(radix 2)
                Naohiro Isshiki    Dec.1995   modified
********************************************************/

int fftr (double *x, double *y, const int m)
{
   int i, j;
   double *xp, *yp, *xq;
   double *yq;
   int mv2, n, tblsize;
   double xt, yt, *sinp, *cosp;
   double arg;

   mv2 = m / 2;

   /* separate even and odd  */
   xq = xp = x;
   yp = y;
   for (i=mv2; --i>=0; ) {
      *xp++ = *xq++;
      *yp++ = *xq++;
   }

   if (fft(x, y, mv2)==-1)        /* m / 2 point fft */
      return(-1);


   /***********************
   * SIN table generation *
   ***********************/

   if ((_sintbl==0) || (maxfftsize<m)) {
      tblsize=m-m/4+1;
      arg=M_PI/m*2;
      if (_sintbl!=0)
         free(_sintbl);
      _sintbl = sinp = dgetmem(tblsize);
      *sinp++ = 0;
      for (j=1; j<tblsize; j++)
         *sinp++ = sin( arg * (double)j);
      _sintbl[m/2] = 0;
      maxfftsize = m;
   }

   n = maxfftsize / m;
   sinp = _sintbl;
   cosp = _sintbl + maxfftsize/4;

   //Z(0) (= Y(0) )
   xp = x;
   yp = y;

   //Z(N) (= Y(N) ) (NB : Z(N) & Y(N) do NOT exist, it is just a programming trick with pointers (so the --xq below) )
   xq = xp + m;
   yq = yp + m;

   //Re(Y(N/2)) = Re(Z(0)) - Im(Z(0))
   *(xp + mv2) = *xp - *yp;
   //Re(Y(0)) = Re(Z(0)) + Im(Z(0))
   *xp = *xp + *yp;
   //Im(Y(O)) = Im(Y(N/2)) = 0
   *(yp + mv2) = *yp = 0;

   for (i=mv2,j=mv2-2; --i ; j-=2) {
      ++xp;//Re(Z(k))
      ++yp;//Im(Z(k))
      sinp += n;//complex exp regularly spaced around the unit circle (it goes n-by-n in case of oversampling of sinp and cosp)
      cosp += n;//idem
      yt = *yp + *(yp + j);//Im(Z(k) + Z(k + N/2-(k*2))) --> Im(Z(k) + Z(N/2-k)), k=1:N/2-1
      xt = *xp - *(xp + j);//Re(Z(k) - Z(N/2-k))
      //fill Y(k) with k= N-1:N/2+1
      *(--xq) = (*xp + *(xp + j) + *cosp * yt - *sinp * xt) * 0.5;//ok
      *(--yq) = (*(yp + j) - *yp + *sinp * yt + *cosp * xt) * 0.5;//-ok --> will be ok (see below)
   }

   xp = x + 1;
   yp = y + 1;
   xq = x + m;
   yq = y + m;

   //copy Y(k) with k=N-1:N/2+1 to Y(k) with k=1:N/2-1
   for (i=mv2; --i; ) {
      *xp++ =   *(--xq);
      *yp++ = -(*(--yq));//-(-ok) == ok
   }

   return(0);
}

/*
 * ifftr code is a modified version of fftr code (from sptk which is BSD-licenced)
 * the licence on the ifftr code is unknown yet (would like to use BSD and send back to sptk).
 *
 * written by Alexis Moinet & Frederic Bettens, TCTS Lab. - FPMs - 2009
 */
int ifftr (double *x, double *y, const int m)
{
   int i, j;
   double *xp, *xq;
   double *yp, *yq;
   int mv2, n, tblsize;
   double xt, yt, *sinp, *cosp;
   double arg;

   mv2 = m / 2;

   /***********************
   * SIN table generation *
   ***********************/

   if ((_sintbl==0) || (maxfftsize<m)) {
      tblsize=m-m/4+1;
      arg=M_PI/m*2;
      if (_sintbl!=0)
         free(_sintbl);
      _sintbl = sinp = dgetmem(tblsize);
      *sinp++ = 0;
      for (j=1; j<tblsize; j++)
         *sinp++ = sin( arg * (double)j);
      _sintbl[m/2] = 0;
      maxfftsize = m;
   }

   n = maxfftsize / m;
   sinp = _sintbl;
   cosp = _sintbl + maxfftsize/4;

   //Z(0) (= Y(0) )
   xp = x;
   yp = y;

   //Z(N) (= Y(N) ) (NB : Z(N) & Y(N) do NOT exist, it is just a programming trick with pointers (so the --xq below) )
   xq = xp + m;
   yq = yp + m;

   //Re(Z(0)) = Re(Z(N/2)) = Re(Y(0)+Y(N/2))
   xt = *xp + *(xp + mv2);
   //Im(Z(O)) = Im(Z(N/2)) = Re(Y(0)-Y(N/2))
   yt = *xp - *(xp + mv2);
   
   *(xp + mv2) = *xp = xt;
   *(yp + mv2) = *yp = yt;

   for (i=mv2,j=mv2-2; --i ; j-=2) {
      ++xp;//Re(Z(k))
      ++yp;//Im(Z(k))
      sinp += n;//complex exp regularly spaced around the unit circle (it goes n-by-n in case of oversampling of sinp and cosp)
      cosp += n;//idem
      yt = *yp + *(yp + j);//Im(Z(k) + Z(k + N/2-(k*2))) --> Im(Z(k) + Z(N/2-k)), k=1:N/2-1
      xt = *xp - *(xp + j);//Re(Z(k) - Z(N/2-k))
      //fill Y(k) with k= N-1:N/2+1
      *(--xq) = (*xp + *(xp + j) - *cosp * yt - *sinp * xt) * 0.5;//ok
      *(--yq) = (*yp - *(yp + j) - *sinp * yt + *cosp * xt) * 0.5;//-ok --> will be ok (see below)
   }

   xp = x + 1;
   yp = y + 1;
   xq = x + m;
   yq = y + m;

   //copy Y(k) with k=N-1:N/2+1 to Y(k) with k=1:N/2-1
   for (i=mv2; --i; ) {
      *xp++ = *(--xq);
      *yp++ = *(--yq);
   }

   if (ifft(x, y, mv2)==-1)        /* m / 2 point fft */
      return(-1);

   /* regroup even and odd  */
   xq = x + m;
   xp = x + mv2;
   yp = y + mv2;
   for (i=mv2; --i>=0; ) {
      *--xq = *--yp;//even
      *--xq = *--xp;//odd
   }

   return(0);
}


