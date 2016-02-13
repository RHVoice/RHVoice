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

#ifndef HTS_VOCODER_C
#define HTS_VOCODER_C

#ifdef __cplusplus
#define HTS_VOCODER_C_START extern "C" {
#define HTS_VOCODER_C_END   }
#else
#define HTS_VOCODER_C_START
#define HTS_VOCODER_C_END
#endif                          /* __CPLUSPLUS */

HTS_VOCODER_C_START;

#include <math.h>               /* for sqrt(),log(),exp(),pow(),cos() */

/* hts_engine libraries */
#include "HTS_hidden.h"

static const double HTS_pade[21] = {
   1.00000000000,
   1.00000000000,
   0.00000000000,
   1.00000000000,
   0.00000000000,
   0.00000000000,
   1.00000000000,
   0.00000000000,
   0.00000000000,
   0.00000000000,
   1.00000000000,
   0.49992730000,
   0.10670050000,
   0.01170221000,
   0.00056562790,
   1.00000000000,
   0.49993910000,
   0.11070980000,
   0.01369984000,
   0.00095648530,
   0.00003041721
};

/* HTS_movem: move memory */
static void HTS_movem(double *a, double *b, const int nitem)
{
   long i = (long) nitem;

   if (a > b)
      while (i--)
         *b++ = *a++;
   else {
      a += i;
      b += i;
      while (i--)
         *--b = *--a;
   }
}

/* HTS_mlsafir: sub functions for MLSA filter */
static double HTS_mlsafir(const double x, const double *b, const int m, const double a, const double aa, double *d)
{
   double y = 0.0;
   int i;

   d[0] = x;
   d[1] = aa * d[0] + a * d[1];

   for (i = 2; i <= m; i++)
      d[i] += a * (d[i + 1] - d[i - 1]);

   for (i = 2; i <= m; i++)
      y += d[i] * b[i];

   for (i = m + 1; i > 1; i--)
      d[i] = d[i - 1];

   return (y);
}

/* HTS_mlsadf1: sub functions for MLSA filter */
static double HTS_mlsadf1(double x, const double *b, const int m, const double a, const double aa, const int pd, double *d, const double *ppade)
{
   double v, out = 0.0, *pt;
   int i;

   pt = &d[pd + 1];

   for (i = pd; i >= 1; i--) {
      d[i] = aa * pt[i - 1] + a * d[i];
      pt[i] = d[i] * b[1];
      v = pt[i] * ppade[i];
      x += (1 & i) ? v : -v;
      out += v;
   }

   pt[0] = x;
   out += x;

   return (out);
}

/* HTS_mlsadf2: sub functions for MLSA filter */
static double HTS_mlsadf2(double x, const double *b, const int m, const double a, const double aa, const int pd, double *d, const double *ppade)
{
   double v, out = 0.0, *pt;
   int i;

   pt = &d[pd * (m + 2)];

   for (i = pd; i >= 1; i--) {
      pt[i] = HTS_mlsafir(pt[i - 1], b, m, a, aa, &d[(i - 1) * (m + 2)]);
      v = pt[i] * ppade[i];

      x += (1 & i) ? v : -v;
      out += v;
   }

   pt[0] = x;
   out += x;

   return (out);
}

/* HTS_mlsadf: functions for MLSA filter */
static double HTS_mlsadf(double x, const double *b, const int m, const double a, const int pd, double *d)
{
   const double aa = 1 - a * a;
   const double *ppade = &(HTS_pade[pd * (pd + 1) / 2]);

   x = HTS_mlsadf1(x, b, m, a, aa, pd, d, ppade);
   x = HTS_mlsadf2(x, b, m, a, aa, pd, &d[2 * (pd + 1)], ppade);

   return (x);
}

/* HTS_rnd: functions for random noise generation */
static double HTS_rnd(unsigned long *next)
{
   double r;

   *next = *next * 1103515245L + 12345;
   r = (*next / 65536L) % 32768L;

   return (r / RANDMAX);
}

/* HTS_nrandom: functions for gaussian random noise generation */
static double HTS_nrandom(HTS_Vocoder * v)
{
   if (v->sw == 0) {
      v->sw = 1;
      do {
         v->r1 = 2 * HTS_rnd(&v->next) - 1;
         v->r2 = 2 * HTS_rnd(&v->next) - 1;
         v->s = v->r1 * v->r1 + v->r2 * v->r2;
      } while (v->s > 1 || v->s == 0);
      v->s = sqrt(-2 * log(v->s) / v->s);
      return (v->r1 * v->s);
   } else {
      v->sw = 0;
      return (v->r2 * v->s);
   }
}

/* HTS_mceq: function for M-sequence random noise generation */
static int HTS_mseq(HTS_Vocoder * v)
{
   int x0, x28;

   v->x >>= 1;
   if (v->x & B0)
      x0 = 1;
   else
      x0 = -1;
   if (v->x & B28)
      x28 = 1;
   else
      x28 = -1;
   if (x0 + x28)
      v->x &= B31_;
   else
      v->x |= B31;

   return (x0);
}

/* HTS_mc2b: transform mel-cepstrum to MLSA digital fillter coefficients */
static void HTS_mc2b(double *mc, double *b, int m, const double a)
{
   if (mc != b) {
      if (a != 0.0) {
         b[m] = mc[m];
         for (m--; m >= 0; m--)
            b[m] = mc[m] - a * b[m + 1];
      } else
         HTS_movem(mc, b, m + 1);
   } else if (a != 0.0)
      for (m--; m >= 0; m--)
         b[m] -= a * b[m + 1];
}

/* HTS_b2bc: transform MLSA digital filter coefficients to mel-cepstrum */
static void HTS_b2mc(const double *b, double *mc, int m, const double a)
{
   double d, o;

   d = mc[m] = b[m];
   for (m--; m >= 0; m--) {
      o = b[m] + a * d;
      d = b[m];
      mc[m] = o;
   }
}

/* HTS_freqt: frequency transformation */
static void HTS_freqt(HTS_Vocoder * v, const double *c1, const int m1, double *c2, const int m2, const double a)
{
   int i, j;
   const double b = 1 - a * a;
   double *g;

   if (m2 > v->freqt_size) {
      if (v->freqt_buff != NULL)
         HTS_free(v->freqt_buff);
      v->freqt_buff = (double *) HTS_calloc(m2 + m2 + 2, sizeof(double));
      v->freqt_size = m2;
   }
   g = v->freqt_buff + v->freqt_size + 1;

   for (i = 0; i < m2 + 1; i++)
      g[i] = 0.0;

   for (i = -m1; i <= 0; i++) {
      if (0 <= m2)
         g[0] = c1[-i] + a * (v->freqt_buff[0] = g[0]);
      if (1 <= m2)
         g[1] = b * v->freqt_buff[0] + a * (v->freqt_buff[1] = g[1]);
      for (j = 2; j <= m2; j++)
         g[j] = v->freqt_buff[j - 1] + a * ((v->freqt_buff[j] = g[j]) - g[j - 1]);
   }

   HTS_movem(g, c2, m2 + 1);
}

/* HTS_c2ir: The minimum phase impulse response is evaluated from the minimum phase cepstrum */
static void HTS_c2ir(const double *c, const int nc, double *h, const int leng)
{
   int n, k, upl;
   double d;

   h[0] = exp(c[0]);
   for (n = 1; n < leng; n++) {
      d = 0;
      upl = (n >= nc) ? nc - 1 : n;
      for (k = 1; k <= upl; k++)
         d += k * c[k] * h[n - k];
      h[n] = d / n;
   }
}

/* HTS_b2en: calculate frame energy */
static double HTS_b2en(HTS_Vocoder * v, const double *b, const int m, const double a)
{
   int i;
   double en = 0.0;
   double *cep;
   double *ir;

   if (v->spectrum2en_size < m) {
      if (v->spectrum2en_buff != NULL)
         HTS_free(v->spectrum2en_buff);
      v->spectrum2en_buff = (double *) HTS_calloc((m + 1) + 2 * IRLENG, sizeof(double));
      v->spectrum2en_size = m;
   }
   cep = v->spectrum2en_buff + m + 1;
   ir = cep + IRLENG;

   HTS_b2mc(b, v->spectrum2en_buff, m, a);
   HTS_freqt(v, v->spectrum2en_buff, m, cep, IRLENG - 1, -a);
   HTS_c2ir(cep, IRLENG, ir, IRLENG);

   for (i = 0; i < IRLENG; i++)
      en += ir[i] * ir[i];

   return (en);
}

/* HTS_ignorm: inverse gain normalization */
static void HTS_ignorm(double *c1, double *c2, int m, const double g)
{
   double k;
   if (g != 0.0) {
      k = pow(c1[0], g);
      for (; m >= 1; m--)
         c2[m] = k * c1[m];
      c2[0] = (k - 1.0) / g;
   } else {
      HTS_movem(&c1[1], &c2[1], m);
      c2[0] = log(c1[0]);
   }
}

/* HTS_gnorm: gain normalization */
static void HTS_gnorm(double *c1, double *c2, int m, const double g)
{
   double k;
   if (g != 0.0) {
      k = 1.0 + g * c1[0];
      for (; m >= 1; m--)
         c2[m] = c1[m] / k;
      c2[0] = pow(k, 1.0 / g);
   } else {
      HTS_movem(&c1[1], &c2[1], m);
      c2[0] = exp(c1[0]);
   }
}

/* HTS_lsp2lpc: transform LSP to LPC */
static void HTS_lsp2lpc(HTS_Vocoder * v, double *lsp, double *a, const int m)
{
   int i, k, mh1, mh2, flag_odd;
   double xx, xf, xff;
   double *p, *q;
   double *a0, *a1, *a2, *b0, *b1, *b2;

   flag_odd = 0;
   if (m % 2 == 0)
      mh1 = mh2 = m / 2;
   else {
      mh1 = (m + 1) / 2;
      mh2 = (m - 1) / 2;
      flag_odd = 1;
   }

   if (m > v->lsp2lpc_size) {
      if (v->lsp2lpc_buff != NULL)
         HTS_free(v->lsp2lpc_buff);
      v->lsp2lpc_buff = (double *) HTS_calloc(5 * m + 6, sizeof(double));
      v->lsp2lpc_size = m;
   }
   p = v->lsp2lpc_buff + m;
   q = p + mh1;
   a0 = q + mh2;
   a1 = a0 + (mh1 + 1);
   a2 = a1 + (mh1 + 1);
   b0 = a2 + (mh1 + 1);
   b1 = b0 + (mh2 + 1);
   b2 = b1 + (mh2 + 1);

   HTS_movem(lsp, v->lsp2lpc_buff, m);

   for (i = 0; i < mh1 + 1; i++)
      a0[i] = 0.0;
   for (i = 0; i < mh1 + 1; i++)
      a1[i] = 0.0;
   for (i = 0; i < mh1 + 1; i++)
      a2[i] = 0.0;
   for (i = 0; i < mh2 + 1; i++)
      b0[i] = 0.0;
   for (i = 0; i < mh2 + 1; i++)
      b1[i] = 0.0;
   for (i = 0; i < mh2 + 1; i++)
      b2[i] = 0.0;

   /* lsp filter parameters */
   for (i = k = 0; i < mh1; i++, k += 2)
      p[i] = -2.0 * cos(v->lsp2lpc_buff[k]);
   for (i = k = 0; i < mh2; i++, k += 2)
      q[i] = -2.0 * cos(v->lsp2lpc_buff[k + 1]);

   /* impulse response of analysis filter */
   xx = 1.0;
   xf = xff = 0.0;

   for (k = 0; k <= m; k++) {
      if (flag_odd) {
         a0[0] = xx;
         b0[0] = xx - xff;
         xff = xf;
         xf = xx;
      } else {
         a0[0] = xx + xf;
         b0[0] = xx - xf;
         xf = xx;
      }

      for (i = 0; i < mh1; i++) {
         a0[i + 1] = a0[i] + p[i] * a1[i] + a2[i];
         a2[i] = a1[i];
         a1[i] = a0[i];
      }

      for (i = 0; i < mh2; i++) {
         b0[i + 1] = b0[i] + q[i] * b1[i] + b2[i];
         b2[i] = b1[i];
         b1[i] = b0[i];
      }

      if (k != 0)
         a[k - 1] = -0.5 * (a0[mh1] + b0[mh2]);
      xx = 0.0;
   }

   for (i = m - 1; i >= 0; i--)
      a[i + 1] = -a[i];
   a[0] = 1.0;
}

/* HTS_gc2gc: generalized cepstral transformation */
static void HTS_gc2gc(HTS_Vocoder * v, double *c1, const int m1, const double g1, double *c2, const int m2, const double g2)
{
   int i, min, k, mk;
   double ss1, ss2, cc;

   if (m1 > v->gc2gc_size) {
      if (v->gc2gc_buff != NULL)
         HTS_free(v->gc2gc_buff);
      v->gc2gc_buff = (double *) HTS_calloc(m1 + 1, sizeof(double));
      v->gc2gc_size = m1;
   }

   HTS_movem(c1, v->gc2gc_buff, m1 + 1);

   c2[0] = v->gc2gc_buff[0];
   for (i = 1; i <= m2; i++) {
      ss1 = ss2 = 0.0;
      min = m1 < i ? m1 : i - 1;
      for (k = 1; k <= min; k++) {
         mk = i - k;
         cc = v->gc2gc_buff[k] * c2[mk];
         ss2 += k * cc;
         ss1 += mk * cc;
      }

      if (i <= m1)
         c2[i] = v->gc2gc_buff[i] + (g2 * ss2 - g1 * ss1) / i;
      else
         c2[i] = (g2 * ss2 - g1 * ss1) / i;
   }
}

/* HTS_mgc2mgc: frequency and generalized cepstral transformation */
static void HTS_mgc2mgc(HTS_Vocoder * v, double *c1, const int m1, const double a1, const double g1, double *c2, const int m2, const double a2, const double g2)
{
   double a;

   if (a1 == a2) {
      HTS_gnorm(c1, c1, m1, g1);
      HTS_gc2gc(v, c1, m1, g1, c2, m2, g2);
      HTS_ignorm(c2, c2, m2, g2);
   } else {
      a = (a2 - a1) / (1 - a1 * a2);
      HTS_freqt(v, c1, m1, c2, m2, a);
      HTS_gnorm(c2, c2, m2, g1);
      HTS_gc2gc(v, c2, m2, g1, c2, m2, g2);
      HTS_ignorm(c2, c2, m2, g2);
   }
}

/* HTS_lsp2mgc: transform LSP to MGC */
static void HTS_lsp2mgc(HTS_Vocoder * v, double *lsp, double *mgc, const int m, const double alpha)
{
   int i;
   /* lsp2lpc */
   HTS_lsp2lpc(v, lsp + 1, mgc, m);
   if (v->use_log_gain)
      mgc[0] = exp(lsp[0]);
   else
      mgc[0] = lsp[0];

   /* mgc2mgc */
   if (NORMFLG1)
      HTS_ignorm(mgc, mgc, m, v->gamma);
   else if (MULGFLG1)
      mgc[0] = (1.0 - mgc[0]) * ((double) v->stage);
   if (MULGFLG1)
      for (i = m; i >= 1; i--)
         mgc[i] *= -((double) v->stage);
   HTS_mgc2mgc(v, mgc, m, alpha, v->gamma, mgc, m, alpha, v->gamma);
   if (NORMFLG2)
      HTS_gnorm(mgc, mgc, m, v->gamma);
   else if (MULGFLG2)
      mgc[0] = mgc[0] * v->gamma + 1.0;
   if (MULGFLG2)
      for (i = m; i >= 1; i--)
         mgc[i] *= v->gamma;
}

/* HTS_mglsadff: sub functions for MGLSA filter */
static double HTS_mglsadff(double x, const double *b, const int m, const double a, double *d)
{
   int i;

   double y;
   y = d[0] * b[1];
   for (i = 1; i < m; i++) {
      d[i] += a * (d[i + 1] - d[i - 1]);
      y += d[i] * b[i + 1];
   }
   x -= y;

   for (i = m; i > 0; i--)
      d[i] = d[i - 1];
   d[0] = a * d[0] + (1 - a * a) * x;
   return x;
}

/* HTS_mglsadf: sub functions for MGLSA filter */
static double HTS_mglsadf(double x, const double *b, const int m, const double a, const int n, double *d)
{
   int i;

   for (i = 0; i < n; i++)
      x = HTS_mglsadff(x, b, m, a, &d[i * (m + 1)]);

   return x;
}

/* THS_check_lsp_stability: check LSP stability */
static void HTS_check_lsp_stability(double *lsp, size_t m)
{
   size_t i, j;
   double tmp;
   double min = (CHECK_LSP_STABILITY_MIN * PI) / (m + 1);
   HTS_Boolean find;

   for (i = 0; i < CHECK_LSP_STABILITY_NUM; i++) {
      find = FALSE;

      for (j = 1; j < m; j++) {
         tmp = lsp[j + 1] - lsp[j];
         if (tmp < min) {
            lsp[j] -= 0.5 * (min - tmp);
            lsp[j + 1] += 0.5 * (min - tmp);
            find = TRUE;
         }
      }

      if (lsp[1] < min) {
         lsp[1] = min;
         find = TRUE;
      }
      if (lsp[m] > PI - min) {
         lsp[m] = PI - min;
         find = TRUE;
      }

      if (find == FALSE)
         break;
   }
}

/* HTS_lsp2en: calculate frame energy */
static double HTS_lsp2en(HTS_Vocoder * v, double *lsp, size_t m, double alpha)
{
   size_t i;
   double en = 0.0;
   double *buff;

   if (v->spectrum2en_size < m) {
      if (v->spectrum2en_buff != NULL)
         HTS_free(v->spectrum2en_buff);
      v->spectrum2en_buff = (double *) HTS_calloc(m + 1 + IRLENG, sizeof(double));
      v->spectrum2en_size = m;
   }
   buff = v->spectrum2en_buff + m + 1;

   /* lsp2lpc */
   HTS_lsp2lpc(v, lsp + 1, v->spectrum2en_buff, m);
   if (v->use_log_gain)
      v->spectrum2en_buff[0] = exp(lsp[0]);
   else
      v->spectrum2en_buff[0] = lsp[0];

   /* mgc2mgc */
   if (NORMFLG1)
      HTS_ignorm(v->spectrum2en_buff, v->spectrum2en_buff, m, v->gamma);
   else if (MULGFLG1)
      v->spectrum2en_buff[0] = (1.0 - v->spectrum2en_buff[0]) * ((double) v->stage);
   if (MULGFLG1)
      for (i = 1; i <= m; i++)
         v->spectrum2en_buff[i] *= -((double) v->stage);
   HTS_mgc2mgc(v, v->spectrum2en_buff, m, alpha, v->gamma, buff, IRLENG - 1, 0.0, 1);

   for (i = 0; i < IRLENG; i++)
      en += buff[i] * buff[i];
   return en;
}

/* HTS_white_noise: return white noise */
static double HTS_white_noise(HTS_Vocoder * v)
{
   if (v->gauss)
      return (double) HTS_nrandom(v);
   else
      return (double) HTS_mseq(v);
}

/* HTS_Vocoder_initialize_excitation: initialize excitation */
static void HTS_Vocoder_initialize_excitation(HTS_Vocoder * v, double pitch, size_t nlpf)
{
   size_t i;

   v->pitch_of_curr_point = pitch;
   v->pitch_counter = pitch;
   v->pitch_inc_per_point = 0.0;
   if (nlpf > 0) {
      v->excite_buff_size = nlpf;
      v->excite_ring_buff = (double *) HTS_calloc(v->excite_buff_size, sizeof(double));
      for (i = 0; i < v->excite_buff_size; i++)
         v->excite_ring_buff[i] = 0.0;
      v->excite_buff_index = 0;
   } else {
      v->excite_buff_size = 0;
      v->excite_ring_buff = NULL;
      v->excite_buff_index = 0;
   }
}

/* HTS_Vocoder_start_excitation: start excitation of each frame */
static void HTS_Vocoder_start_excitation(HTS_Vocoder * v, double pitch)
{
   if (v->pitch_of_curr_point != 0.0 && pitch != 0.0) {
      v->pitch_inc_per_point = (pitch - v->pitch_of_curr_point) / v->fprd;
   } else {
      v->pitch_inc_per_point = 0.0;
      v->pitch_of_curr_point = pitch;
      v->pitch_counter = pitch;
   }
}

/* HTS_Vocoder_excite_unvoiced_frame: ping noise to ring buffer */
static void HTS_Vocoder_excite_unvoiced_frame(HTS_Vocoder * v, double noise)
{
   size_t center = (v->excite_buff_size - 1) / 2;
   v->excite_ring_buff[(v->excite_buff_index + center) % v->excite_buff_size] += noise;
}

/* HTS_Vocoder_excite_vooiced_frame: ping noise and pulse to ring buffer */
static void HTS_Vocoder_excite_voiced_frame(HTS_Vocoder * v, double noise, double pulse, const double *lpf)
{
   size_t i;
   size_t center = (v->excite_buff_size - 1) / 2;

   if (noise != 0.0) {
      for (i = 0; i < v->excite_buff_size; i++) {
         if (i == center)
            v->excite_ring_buff[(v->excite_buff_index + i) % v->excite_buff_size] += noise * (1.0 - lpf[i]);
         else
            v->excite_ring_buff[(v->excite_buff_index + i) % v->excite_buff_size] += noise * (0.0 - lpf[i]);
      }
   }
   if (pulse != 0.0) {
      for (i = 0; i < v->excite_buff_size; i++)
         v->excite_ring_buff[(v->excite_buff_index + i) % v->excite_buff_size] += pulse * lpf[i];
   }
}

/* HTS_Vocoder_get_excitation: get excitation of each sample */
static double HTS_Vocoder_get_excitation(HTS_Vocoder * v, const double *lpf)
{
   double x;
   double noise, pulse = 0.0;

   if (v->excite_buff_size > 0) {
      noise = HTS_white_noise(v);
      pulse = 0.0;
      if (v->pitch_of_curr_point == 0.0) {
         HTS_Vocoder_excite_unvoiced_frame(v, noise);
      } else {
         v->pitch_counter += 1.0;
         if (v->pitch_counter >= v->pitch_of_curr_point) {
            pulse = sqrt(v->pitch_of_curr_point);
            v->pitch_counter -= v->pitch_of_curr_point;
         }
         HTS_Vocoder_excite_voiced_frame(v, noise, pulse, lpf);
         v->pitch_of_curr_point += v->pitch_inc_per_point;
      }
      x = v->excite_ring_buff[v->excite_buff_index];
      v->excite_ring_buff[v->excite_buff_index] = 0.0;
      v->excite_buff_index++;
      if (v->excite_buff_index >= v->excite_buff_size)
         v->excite_buff_index = 0;
   } else {
      if (v->pitch_of_curr_point == 0.0) {
         x = HTS_white_noise(v);
      } else {
         v->pitch_counter += 1.0;
         if (v->pitch_counter >= v->pitch_of_curr_point) {
            x = sqrt(v->pitch_of_curr_point);
            v->pitch_counter -= v->pitch_of_curr_point;
         } else {
            x = 0.0;
         }
         v->pitch_of_curr_point += v->pitch_inc_per_point;
      }
   }

   return x;
}

/* HTS_Vocoder_end_excitation: end excitation of each frame */
static void HTS_Vocoder_end_excitation(HTS_Vocoder * v, double pitch)
{
   v->pitch_of_curr_point = pitch;
}

/* HTS_Vocoder_postfilter_mcp: postfilter for MCP */
static void HTS_Vocoder_postfilter_mcp(HTS_Vocoder * v, double *mcp, const int m, double alpha, double beta)
{
   double e1, e2;
   int k;

   if (beta > 0.0 && m > 1) {
      if (v->postfilter_size < m) {
         if (v->postfilter_buff != NULL)
            HTS_free(v->postfilter_buff);
         v->postfilter_buff = (double *) HTS_calloc(m + 1, sizeof(double));
         v->postfilter_size = m;
      }
      HTS_mc2b(mcp, v->postfilter_buff, m, alpha);
      e1 = HTS_b2en(v, v->postfilter_buff, m, alpha);

      v->postfilter_buff[1] -= beta * alpha * v->postfilter_buff[2];
      for (k = 2; k <= m; k++)
         v->postfilter_buff[k] *= (1.0 + beta);

      e2 = HTS_b2en(v, v->postfilter_buff, m, alpha);
      v->postfilter_buff[0] += log(e1 / e2) / 2;
      HTS_b2mc(v->postfilter_buff, mcp, m, alpha);
   }
}

/* HTS_Vocoder_postfilter_lsp: postfilter for LSP */
static void HTS_Vocoder_postfilter_lsp(HTS_Vocoder * v, double *lsp, size_t m, double alpha, double beta)
{
   double e1, e2;
   size_t i;
   double d1, d2;

   if (beta > 0.0 && m > 1) {
      if (v->postfilter_size < m) {
         if (v->postfilter_buff != NULL)
            HTS_free(v->postfilter_buff);
         v->postfilter_buff = (double *) HTS_calloc(m + 1, sizeof(double));
         v->postfilter_size = m;
      }

      e1 = HTS_lsp2en(v, lsp, m, alpha);

      /* postfiltering */
      for (i = 0; i <= m; i++) {
         if (i > 1 && i < m) {
            d1 = beta * (lsp[i + 1] - lsp[i]);
            d2 = beta * (lsp[i] - lsp[i - 1]);
            v->postfilter_buff[i] = lsp[i - 1] + d2 + (d2 * d2 * ((lsp[i + 1] - lsp[i - 1]) - (d1 + d2))) / ((d2 * d2) + (d1 * d1));
         } else {
            v->postfilter_buff[i] = lsp[i];
         }
      }
      HTS_movem(v->postfilter_buff, lsp, m + 1);

      e2 = HTS_lsp2en(v, lsp, m, alpha);

      if (e1 != e2) {
         if (v->use_log_gain)
            lsp[0] += 0.5 * log(e1 / e2);
         else
            lsp[0] *= sqrt(e1 / e2);
      }
   }
}

/* HTS_Vocoder_initialize: initialize vocoder */
void HTS_Vocoder_initialize(HTS_Vocoder * v, size_t m, size_t stage, HTS_Boolean use_log_gain, size_t rate, size_t fperiod)
{
   /* set parameter */
   v->is_first = TRUE;
   v->stage = stage;
   if (stage != 0)
      v->gamma = -1.0 / v->stage;
   else
      v->gamma = 0.0;
   v->use_log_gain = use_log_gain;
   v->fprd = fperiod;
   v->next = SEED;
   v->gauss = GAUSS;
   v->rate = rate;
   v->pitch_of_curr_point = 0.0;
   v->pitch_counter = 0.0;
   v->pitch_inc_per_point = 0.0;
   v->excite_ring_buff = NULL;
   v->excite_buff_size = 0;
   v->excite_buff_index = 0;
   v->sw = 0;
   v->x = 0x55555555;
   /* init buffer */
   v->freqt_buff = NULL;
   v->freqt_size = 0;
   v->gc2gc_buff = NULL;
   v->gc2gc_size = 0;
   v->lsp2lpc_buff = NULL;
   v->lsp2lpc_size = 0;
   v->postfilter_buff = NULL;
   v->postfilter_size = 0;
   v->spectrum2en_buff = NULL;
   v->spectrum2en_size = 0;
   if (v->stage == 0) {         /* for MCP */
      v->c = (double *) HTS_calloc(m * (3 + PADEORDER) + 5 * PADEORDER + 6, sizeof(double));
      v->cc = v->c + m + 1;
      v->cinc = v->cc + m + 1;
      v->d1 = v->cinc + m + 1;
   } else {                     /* for LSP */
      v->c = (double *) HTS_calloc((m + 1) * (v->stage + 3), sizeof(double));
      v->cc = v->c + m + 1;
      v->cinc = v->cc + m + 1;
      v->d1 = v->cinc + m + 1;
   }
}

/* HTS_Vocoder_synthesize: pulse/noise excitation and MLSA/MGLSA filster based waveform synthesis */
void HTS_Vocoder_synthesize(HTS_Vocoder * v, size_t m, double lf0, double *spectrum, size_t nlpf, double *lpf, double alpha, double beta, double volume, double *rawdata, HTS_Audio * audio)
{
   double x;
   int i, j;
   short xs;
   int rawidx = 0;
   double p;

   /* lf0 -> pitch */
   if (lf0 == LZERO)
      p = 0.0;
   else if (lf0 <= MIN_LF0)
      p = v->rate / MIN_F0;
   else if (lf0 >= MAX_LF0)
      p = v->rate / MAX_F0;
   else
      p = v->rate / exp(lf0);

   /* first time */
   if (v->is_first == TRUE) {
      HTS_Vocoder_initialize_excitation(v, p, nlpf);
      if (v->stage == 0) {      /* for MCP */
         HTS_mc2b(spectrum, v->c, m, alpha);
      } else {                  /* for LSP */
         HTS_movem(spectrum, v->c, m + 1);
         HTS_lsp2mgc(v, v->c, v->c, m, alpha);
         HTS_mc2b(v->c, v->c, m, alpha);
         HTS_gnorm(v->c, v->c, m, v->gamma);
         for (i = 1; i <= m; i++)
            v->c[i] *= v->gamma;
      }
      v->is_first = FALSE;
   }

   HTS_Vocoder_start_excitation(v, p);
   if (v->stage == 0) {         /* for MCP */
      HTS_Vocoder_postfilter_mcp(v, spectrum, m, alpha, beta);
      HTS_mc2b(spectrum, v->cc, m, alpha);
      for (i = 0; i <= m; i++)
         v->cinc[i] = (v->cc[i] - v->c[i]) / v->fprd;
   } else {                     /* for LSP */
      HTS_Vocoder_postfilter_lsp(v, spectrum, m, alpha, beta);
      HTS_check_lsp_stability(spectrum, m);
      HTS_lsp2mgc(v, spectrum, v->cc, m, alpha);
      HTS_mc2b(v->cc, v->cc, m, alpha);
      HTS_gnorm(v->cc, v->cc, m, v->gamma);
      for (i = 1; i <= m; i++)
         v->cc[i] *= v->gamma;
      for (i = 0; i <= m; i++)
         v->cinc[i] = (v->cc[i] - v->c[i]) / v->fprd;
   }

   for (j = 0; j < v->fprd; j++) {
      x = HTS_Vocoder_get_excitation(v, lpf);
      if (v->stage == 0) {      /* for MCP */
         if (x != 0.0)
            x *= exp(v->c[0]);
         x = HTS_mlsadf(x, v->c, m, alpha, PADEORDER, v->d1);
      } else {                  /* for LSP */
         if (!NGAIN)
            x *= v->c[0];
         x = HTS_mglsadf(x, v->c, m, alpha, v->stage, v->d1);
      }
      x *= volume;

      /* output */
      if (rawdata)
         rawdata[rawidx++] = x;
      if (audio) {
         if (x > 32767.0)
            xs = 32767;
         else if (x < -32768.0)
            xs = -32768;
         else
            xs = (short) x;
         HTS_Audio_write(audio, xs);
      }

      for (i = 0; i <= m; i++)
         v->c[i] += v->cinc[i];
   }

   HTS_Vocoder_end_excitation(v, p);
   HTS_movem(v->cc, v->c, m + 1);
}

/* HTS_Vocoder_clear: clear vocoder */
void HTS_Vocoder_clear(HTS_Vocoder * v)
{
   if (v != NULL) {
      /* free buffer */
      if (v->freqt_buff != NULL) {
         HTS_free(v->freqt_buff);
         v->freqt_buff = NULL;
      }
      v->freqt_size = 0;
      if (v->gc2gc_buff != NULL) {
         HTS_free(v->gc2gc_buff);
         v->gc2gc_buff = NULL;
      }
      v->gc2gc_size = 0;
      if (v->lsp2lpc_buff != NULL) {
         HTS_free(v->lsp2lpc_buff);
         v->lsp2lpc_buff = NULL;
      }
      v->lsp2lpc_size = 0;
      if (v->postfilter_buff != NULL) {
         HTS_free(v->postfilter_buff);
         v->postfilter_buff = NULL;
      }
      v->postfilter_size = 0;
      if (v->spectrum2en_buff != NULL) {
         HTS_free(v->spectrum2en_buff);
         v->spectrum2en_buff = NULL;
      }
      v->spectrum2en_size = 0;
      if (v->c != NULL) {
         HTS_free(v->c);
         v->c = NULL;
      }
      v->excite_buff_size = 0;
      v->excite_buff_index = 0;
      if (v->excite_ring_buff != NULL) {
         HTS_free(v->excite_ring_buff);
         v->excite_ring_buff = NULL;
      }
   }
}

HTS_VOCODER_C_END;

#endif                          /* !HTS_VOCODER_C */
