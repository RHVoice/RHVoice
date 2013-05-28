 /* ----------------------------------------------------------------------------------------------- */
 /* 																								*/
 /* 	This file is part of MAGE / pHTS( the performative HMM-based speech synthesis system )		*/
 /* 																								*/
 /* 	MAGE / pHTS is free software: you can redistribute it and/or modify it under the terms		*/
 /* 	of the GNU General Public License as published by the Free Software Foundation, either		*/
 /* 	version 3 of the license, or any later version.												*/
 /* 																								*/
 /* 	MAGE / pHTS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;	*/	
 /* 	without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	*/
 /* 	See the GNU General Public License for more details.										*/
 /* 																								*/	
 /* 	You should have received a copy of the GNU General Public License along with MAGE / pHTS.	*/ 
 /* 	If not, see http://www.gnu.org/licenses/													*/
 /* 																								*/
 /* 																								*/	
 /* 	Copyright 2011 University of Mons :															*/
 /* 																								*/	
 /* 			Numediart Institute for New Media Art( www.numediart.org )							*/
 /* 			Acapela Group ( www.acapela-group.com )												*/
 /* 																								*/
 /* 																								*/
 /* 	 Developed by :																				*/
 /* 																								*/
 /* 		Maria Astrinaki, Alexis Moinet, Geoffrey Wilfart, Nicolas d'Alessandro, Thierry Dutoit	*/
 /* 																								*/
 /* ----------------------------------------------------------------------------------------------- */

/* 
 	Most of these functions come from SPTK, which is distributed under 
 	a Modified BSD License. See http://sp-tk.sourceforge.net/ for details

 	SPTK's Modified BSD license is also included in this file, just above
 	the set of functions that we copy/pasted and adapted from SPTK.
*/

#include "Vocoder.h"

//	Constructor that allocates and initializes all the parameters needed for the Vocoder.
MAGE::Vocoder::Vocoder( int am, double aalpha, int afprd, int aiprd, int astage, int apd, bool angain )
{
	this->m = am;			// nOfMGCs-1;
	this->fprd = afprd;		// 240;
	this->iprd = aiprd;		// 1;
	this->stage = astage;	// 0;
	this->pd = apd;			// 4;
	this->ngain = angain;	// true;
	this->alpha = aalpha;	// 0.55;
	
	//excitation
	this->count = 0;
	
	this->action = noaction;
	
	this->f0 = MAGE::defaultPitch;// 110Hz, default pitch
	this->t0 = MAGE::defaultSamplingRate / this->f0; // defaultSamplingRate = 48000
	
	this->voiced = false;
	
	this->volume = 1.0;
	
	if( stage != 0 ) // MGLSA
		gamma = -1 / ( double )stage;
	
	this->csize = 0;
	
	if( stage != 0 )
		this->csize = m + m + m + 3 + ( m + 1 ) * stage;  // MGLSA
	else
		this->csize = 3 * ( m + 1 )+ 3 * ( pd + 1 ) + pd * ( m + 2 );  // MLSA
	
	c = new double[this->csize];
	
	for( int k = 0; k < this->csize; k++ )
		c[k] = 0;
	
	cc = c + m + 1;
	inc = cc + m + 1;
	d = inc + m + 1;
	
	flagFirstPush = true;
	this->nOfPopSinceLastPush = 0;
	
	double lpadesptk[] = 
	{ 
		1.0,		
		1.0,		0.0,			1.0,			0.0,		
		0.0,		1.0,			0.0,			0.0,
		0.0,		1.0,			0.4999273,		0.1067005,
		0.01170221,	0.0005656279,	1.0,			0.4999391,	
		0.1107098,	0.01369984,		0.0009564853,	0.00003041721
	};
	
	for( int k = 0; k < 21; k++ )
		padesptk[k] = lpadesptk[k];
}

//	Constructor that allocates and initializes all the parameters needed for the Vocoder.
MAGE::Vocoder::Vocoder( const Vocoder& orig )
{
}

//	Destructor that disallocates all the memory used from the Vocoder.
MAGE::Vocoder::~Vocoder()
{
	delete[] c;
}

// getters


// setters

//	This function forces the value of the pitch used by the Vocoder instead of the one in Frame (f0).
void MAGE::Vocoder::setPitch( double pitch, int action, bool forceVoiced )
{
	switch( action )
	{
		case MAGE::overwrite:
			this->f0 = pitch; // Hz
			break;
			
		case MAGE::shift:
			this->f0 = ( this->f0 ) + ( pitch ); // Hz
			break;
			
		case MAGE::scale:
			this->f0 = ( this->f0 ) * ( pitch ); // Hz
			break;
			
		case MAGE::synthetic:
		case MAGE::noaction:
			break;
		default:
			this->f0 = pitch;//Hz
	}
	
	this->action = action;
	this->actionValue = pitch;
	
	// ATTENTION!! Should I do that???
	//set a default value for the pitch in case a negative value is entered
	if( this->f0 < 0 )
		this->f0 = MAGE::defaultPitch; 
	
	this->t0 = MAGE::defaultSamplingRate / this->f0;	// defaultSamplingRate = 48000
	
	if( forceVoiced )
		this->voiced = true;
	
	return;
}

//	This function forces a Frame to be voiced or unvoiced.
void MAGE::Vocoder::setVoiced( bool forceVoiced )
{
	this->voiced = forceVoiced;
	return;
}

// methods

//	This function generates a Frame.
void MAGE::Vocoder::push( Frame &frame, bool ignoreVoicing )
{
	int i;
	
	if( !flagFirstPush )
	{
		movem( cc, c, sizeof( * cc ), m + 1 );
		
		mc2b( frame.streams[mgcStreamIndex], cc, m, alpha );
		
		if( stage != 0 ) // MGLSA
		{
			gnorm( cc, cc, m, gamma );
			cc[0] = log( cc[0] );
			
			for( i = 1; i <= m; i++ )
				cc[i] *= gamma;
		}
	} 
	else 
	{
		flagFirstPush = false;
		
		mc2b( frame.streams[mgcStreamIndex], c, m, alpha );
		
		if( stage != 0 ) // MGLSA
		{ 
			gnorm( c, c, m, gamma );
			c[0] = log( c[0] );
			
			for( i = 1; i <= m; i++ )
				c[i] *= gamma;
		}
		
		for( i = 0; i <= m; i++ )
			cc[i] = c[i];
	}	
	
	for( i = 0; i <= m; i++ )
		inc[i] = ( cc[i] - c[i] ) * iprd / fprd;
	
	switch( action )
	{
		case MAGE::overwrite:
			this->f0 = this->actionValue; // Hz
			break;
			
		case MAGE::shift:
			this->f0 = ( frame.streams[lf0StreamIndex][0]  ) + ( this->actionValue ); // Hz
			break;
			
		case MAGE::scale:
			this->f0 = ( frame.streams[lf0StreamIndex][0]  ) * ( this->actionValue );  // Hz
			break;
			
		case MAGE::synthetic:
		case MAGE::noaction:
		default:
			this->f0 = frame.streams[lf0StreamIndex][0] ;
	}
	
	if( this->f0 < 0 )
		this->f0 = MAGE::defaultPitch; 
	
	this->t0 = MAGE::defaultSamplingRate / this->f0; // defaultSamplingRate = 48000
	
	if( !ignoreVoicing )
		this->voiced = frame.voiced;
	
	this->nOfPopSinceLastPush = 0;
	return;
}

//	This function generates a Frame.
void MAGE::Vocoder::push( Frame * frame, bool ignoreVoicing )
{
	int i;
	
	if( !flagFirstPush )
	{
		movem( cc, c, sizeof( * cc ), m + 1 );
		
		mc2b( frame->streams[mgcStreamIndex], cc, m, alpha );
		
		if( stage != 0 ) /* MGLSA*/
		{
			gnorm( cc, cc, m, gamma );
			cc[0] = log( cc[0] );
			
			for( i = 1; i <= m; i++ )
				cc[i] *= gamma;
		}
	} 
	else 
	{
		flagFirstPush = false;
		
		mc2b( frame->streams[mgcStreamIndex], c, m, alpha );
		
		if( stage != 0 ) // MGLSA
		{ 
			gnorm( c, c, m, gamma );
			c[0] = log( c[0] );
			
			for( i = 1; i <= m; i++ )
				c[i] *= gamma;
		}
		
		for( i = 0; i <= m; i++ )
			cc[i] = c[i];
	}	
	
	for( i = 0; i <= m; i++ )
		inc[i] = ( cc[i] - c[i] ) * iprd / fprd;
	
	switch( action )
	{
		case MAGE::overwrite:
			this->f0 = this->actionValue;	// Hz
			break;
			
		case MAGE::shift:
			this->f0 = ( frame->streams[lf0StreamIndex][0]  ) + ( this->actionValue ); // Hz
			break;
			
		case MAGE::scale:
			this->f0 = ( frame->streams[lf0StreamIndex][0]  ) * ( this->actionValue );  // Hz
			break;
			
		case MAGE::synthetic:
		case MAGE::noaction:
		default:
			this->f0 = frame->streams[lf0StreamIndex][0] ;
	}
	
	if( this->f0 < 0 )
		this->f0 = MAGE::defaultPitch; 
	
	this->t0 = MAGE::defaultSamplingRate / this->f0; // defaultSamplingRate = 48000
	
	if( !ignoreVoicing )
		this->voiced = frame->voiced;
	
	this->nOfPopSinceLastPush = 0;
	return;
}
//	This function returns a single sample from the Vocoder.
double MAGE::Vocoder::pop()
{
	int i;
	
	if( voiced )
	{
		if( count <= 0 )
		{
			x = sqrt( this->t0 );
			count = this->t0;
		} 
		else 
		{
			x = 0;
			count--;
		}
	} 
	else
	{
		x = MAGE::Random( -1,1 );
		count = 0;
	}
	
	if( stage != 0 )  // MGLSA
	{ 
		if( !ngain )
			x *= exp( c[0] );
		
		x = mglsadf( x, c, m, alpha, stage, d );
	} 
	else  // MLSA
	{ 
		if( !ngain )
			x *= exp( c[0] );
		
		x = mlsadf( x, c, m, alpha, pd, d );
	}

	//filter interpolation has not reached next filter yet
	//when next filter is reached, stop interpolation, otherwise
	//you'll eventually get an unstable filter
	if( this->nOfPopSinceLastPush < ( fprd/iprd ) ) 
		for( i = 0; i <= m; i++ )
			c[i] += inc[i];
	
	this->nOfPopSinceLastPush++;
	
	// ATTENTION volume??? correct place???
	if( this->volume >= 0 )
		return( this->volume * x );
	
	return( x );
}

//	This function returns true if at least one Frame has been pushed, otherwise false.
bool MAGE::Vocoder::ready()
{ 
	return (!this->flagFirstPush); 
}

//	This function resets the Vocoder to its default values.
void MAGE::Vocoder::reset()
{
	for( int i = 0; i < this->csize; i++ )
		c[i] = 0;
	
	this->f0 = MAGE::defaultPitch;
	this->t0 = MAGE::defaultSamplingRate / ( this->f0 );
	this->action = MAGE::noaction;
	this->alpha  = MAGE::defaultAlpha;
	this->gamma  = MAGE::defaultGamma;
	this->stage  = MAGE::defaultGamma;
	this->pd	 = MAGE::defaultPadeOrder; 
	this->volume = MAGE::defaultVolume;
	this->flagFirstPush = true;
	
	return;
}

// accessors
//	This function returns true if the current Frame is voiced and false otherwise.
bool MAGE::Vocoder::isVoiced( void )
{
	return( this->voiced );
}

// functions imported from SPTK and SPTK's license

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
 $Id: movem.c,v 1.10 2011/04/27 13:46:44 mataki Exp $
 
 Data Transfer Function
 
 movem( a, b, size, nitem )
 
 void	 * a	: intput data
 void	 * b	: output data
 size_t size	: size of data type
 int	nitem : data length
 
*********************************************************/
void MAGE::Vocoder::movem( void * a, void * b, const size_t size, const int nitem )
{
	long i;
	char * c = ( char * )a;
	char * d = ( char * )b;
	
	i = size * nitem;
	if( c > d )
		while( i-- )
			 * d++ = * c++;
	else 
	{
		c += i;
		d += i;
		while( i-- )
			 * --d = * --c;
	}
	return;
}

void MAGE::Vocoder::mc2b( double * mc, double * b, int m, const double a )
{
	b[m] = mc[m];
	
	for( m--; m >= 0; m-- )
		b[m] = mc[m] - a * b[m + 1];
	
	return;
}

void MAGE::Vocoder::gnorm( double * c1, double * c2, int m, const double g )
{
	double k;
	
	if( g != 0.0 )
	{
		k = 1.0 + g * c1[0];
		
		for( ; m >= 1; m-- )
			c2[m] = c1[m] / k;
		
		c2[0] = pow( k, 1.0 / g );
	} 
	else
	{
		movem( &c1[1], &c2[1], sizeof( * c1 ), m );
		c2[0] = exp( c1[0] );
	}	
	return;
}

double MAGE::Vocoder::mglsadff( double x, double * b, const int m, const double a, double * d )
{
	int i;
	double y, aa;
	
	aa = 1 - a * a;
	
	y = d[0] * b[1];
	
	for( i = 1; i < m; i++ )
	{
		d[i] += a * ( d[i + 1] - d[i - 1] );
		y += d[i] * b[i + 1];
	}
	
	x -= y;
	
	for( i = m; i > 0; i-- )
		d[i] = d[i - 1];
	
	d[0] = a * d[0] + aa * x;
	
	return( x );
}

double MAGE::Vocoder::mglsadf( double x, double * b, const int m, const double a, const int n, double * d )
{
	int i;
	
	for( i = 0; i < n; i++ )
		x = mglsadff( x, b, m, a, &d[i * ( m + 1 )] );
	
	return( x );
}

double MAGE::Vocoder::mlsafir( double x, double * b, const int m, const double a, double * d )
{
	double y = 0.0, aa;
	int i;
	
	aa = 1 - a * a;
	
	d[0] = x;
	d[1] = aa * d[0] + a * d[1];
	
	for( i = 2; i <= m; i++ )
	{
		d[i] = d[i] + a * ( d[i + 1] - d[i - 1] );
		y += d[i] * b[i];
	}
	
	for( i = m + 1; i > 1; i-- )
		d[i] = d[i - 1];
	
	return( y );
}

double MAGE::Vocoder::mlsadf1( double x, double * b, const int m, const double a, const int pd, double * d )
{
	double v, out = 0.0, * pt, aa;
	int i;
	
	aa = 1 - a * a;
	pt = &d[pd + 1];
	
	for( i = pd; i >= 1; i-- )
	{
		d[i] = aa * pt[i - 1] + a * d[i];
		pt[i] = d[i] * b[1];
		v = pt[i] * ppadesptk[i];
		
		x += ( 1 & i )? v : -v;
		out += v;
	}
	
	pt[0] = x;
	out += x;
	
	return( out );
}

double MAGE::Vocoder::mlsadf2( double x, double * b, const int m, const double a, const int pd, double * d )
{
	double v, out = 0.0, * pt, aa;
	int i;
	
	aa = 1 - a * a;
	pt = &d[pd * ( m + 2 )];
	
	for( i = pd; i >= 1; i-- )
	{
		pt[i] = mlsafir( pt[i - 1], b, m, a, &d[( i - 1 ) * ( m + 2 )] );
		v = pt[i] * ppadesptk[i];
		
		x += ( 1 & i ) ? v : -v;
		out += v;
	}
	
	pt[0] = x;
	out += x;
	
	return( out );
}

double MAGE::Vocoder::mlsadf( double x, double * b, const int m, const double a, const int pd, double * d )
{
	ppadesptk = &padesptk[pd * ( pd + 1 ) / 2];
	
	x = mlsadf1( x, b, m, a, pd, d );
	x = mlsadf2( x, b, m, a, pd, &d[2 * ( pd + 1 )] );
	
	return( x );
}

