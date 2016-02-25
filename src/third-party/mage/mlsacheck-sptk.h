/* 
 * File:   mlsacheck-sptk.h
 * Author: Maria Astrinaki
 *
 * Created on 09/10/2012, 16:18
 */

#ifndef _MLSACHECK_SPTK_H
#define	_MLSACHECK_SPTK_H

#include <stdio.h>
#include <stdlib.h>

/*  Default Values  */
//#define FFTLENGTH 256
#define STABLE1 0
#define STABLE2 1
#define TR 1

#include "fft-sptk.h"
#include "getmem-sptk.h"

#ifdef	__cplusplus
extern "C" {
#endif
	void mlsacheck(double *mcep, int m, int fftlen, int frame,
				   double a, double R1, double R2,
				   int modify_filter, int stable_condition);
#ifdef	__cplusplus
}
#endif

#endif	/* _MLSACHECK_SPTK_H */

