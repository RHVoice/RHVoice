/* 
 * File:   getmem-sptk.h
 * Author: alexis
 *
 * Created on 21 janvier 2009, 15:18
 */

#ifndef _GETMEM_H
#define	_GETMEM_H

#include <stdio.h>
#include <stdlib.h>

#ifdef DOUBLE
#define real double
#else
#define real float
#endif

#ifdef	__cplusplus
extern "C" {
#endif

char *getmem (const size_t leng, const size_t size);
short *sgetmem (const int leng);
long *lgetmem (const int leng);
double *dgetmem (const int leng);
float *fgetmem (const int leng);
real *rgetmem (const int leng);
float **ffgetmem (const int leng);


#ifdef	__cplusplus
}
#endif

#endif	/* _GETMEM_H */

