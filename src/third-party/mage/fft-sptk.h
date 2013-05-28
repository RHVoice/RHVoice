/* 
 * File:   fft-sptk.h
 * Author: alexis
 *
 * Created on 21 janvier 2009, 14:57
 */

#ifndef _FFT_SPTK_H
#define	_FFT_SPTK_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846 //define pi for windows
#endif /*M_PI*/

#include "getmem-sptk.h"

#ifdef	__cplusplus
extern "C" {
#endif
    static int checkm (const int m);
    int fft(double* x, double* y, const int m);
    int ifft(double* x, double* y, const int m);
    int fftr(double* x, double* y, const int m);
    int ifftr(double* x, double* y, const int m);


#ifdef	__cplusplus
}
#endif

#endif	/* _FFT_SPTK_H */

