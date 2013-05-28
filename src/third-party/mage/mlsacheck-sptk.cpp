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
/*                1996-2012  Nagoya Institute of Technology          */
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

/************************************************************************
 *                                                                       *
 *    Check stability of MLSA filter coefficients                        *
 *                                                                       *
 *                                         2012.9  Akira Tamamori        *
 *                                                                       *
 *       usage:                                                          *
 *               mlsacheck [ options ] [ infile] > stdout                *
 *       options:                                                        *
 *               -m m     :  order of mel-cepstrum               [25]    *
 *               -a a     :  all-pass constant                   [0.35]  *
 *               -l L     :  FFT length                          [256]   *
 *               -c       :  modify MLSA filter coef.            [N/A]   *
 *               -r R     :  stable condition for MLSA filter    [N/A]   *
 *               -P P     :  order of Pade approximation         [4]     *
 *       infile:                                                         *
 *               mel cepstral coefficients                               *
 *       stdout:                                                         *
 *               mel-cepstrums satisfying stability condition            *
 *       notice:                                                         *
 *               P = 4 or 5                                              *
 *                                                                       *
 ************************************************************************/

#include "mlsacheck-sptk.h"

void mlsacheck(double *mcep, int m, int fftlen, int frame,
               double a, double R1, double R2,
               int modify_filter, int stable_condition)
{
	int i;
	double K, r, *x, *y, *mag;
	
	x = dgetmem(fftlen);
	y = dgetmem(fftlen);
	mag = dgetmem(fftlen / 2);
	
	for (i = 0; i < fftlen; i++) 
	{
		x[i] = 0;
		y[i] = 0;
	}
	
	for (i = 0; i < m + 1; i++) 
		x[i] = mcep[i];
	
	/* calculate gain factor */
	for (i = 0, K = 0.0; i < m + 1; i++) 
		K += x[i] * pow(a, i);
	
	/* gain normalization */
	x[0] -= K;
	
	fftr(x, y, fftlen);
	
	/* check stability */
	for (i = 0; i < fftlen / 2; i++) 
	{
		mag[i] = x[i] * x[i] + y[i] * y[i];
		mag[i] = sqrt(mag[i]);
		
		if (mag[i] > R1 || mag[i] > R2)  /* unstable */
		{
			/* output ascii report */
		//	fprintf(stderr, "[ unstable frame number : %d ]\n", frame);
		
		//	for (i = 0; i < m + 1; i++) 
		//		fprintf(stderr, "%f\n", mcep[i]);
			
			if (modify_filter == TR)    /* -c option */
			{
				switch (stable_condition)  /* -r option */
				{
					case STABLE1:
						
						if (mag[i] > R1) 
						{
							r = R1 / mag[i];
							x[i] *= r;
							y[i] *= r;
							x[fftlen - 1 - i] *= r;
							y[fftlen - 1 - i] *= r;
						}
						break;
						
					case STABLE2:
						
						if (mag[i] > R2) 
						{
							r = R2 / mag[i];
							x[i] *= r;
							y[i] *= r;
							x[fftlen - 1 - i] *= r;
							y[fftlen - 1 - i] *= r;
						}
				}
			}
		}
	}
	
	ifft(x, y, fftlen);
	
	/* revert gain factor */
	x[0] += K;
	
	for (i = 0; i < m + 1; i++) 
		mcep[i] = x[i];

	// fwrite(x, sizeof(*x), m + 1, stdout);
	
	free(x);
	free(y);
	free(mag);
	
	return;
}

/*
 switch (pd) {
 case 4:
 R1 = 4.5;
 R2 = 6.2;
 break;
 case 5:
 R1 = 6.0;
 R2 = 7.65;
 break;
 default:
 fprintf(stderr, "%s : Order of Pade approximation should be 4 or 5!\n",
 cmnd);
 usage(1);
 break;
 }
 
 mcep = dgetmem(m + 1);
 
 //check stability of MLFA filter and output 
 while (freadf(mcep, sizeof(*mcep), m + 1, fp) == m + 1) {
 mlsacheck(mcep, m, fftlen, frame, a, R1, R2,
 modify_filter, stable_condition);
 frame++;
 }
 
 return (0);
 }*/
