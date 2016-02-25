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
 */

#pragma once

#include <cstdio>

#include "Constants.h"
#include "Frame.h"
#include "MathFunctions.h"

namespace MAGE 
{
	/** 
	 *  \brief		Definition of the Vocoder.
	 *  \details	This class is used to define the Vocoder used to synthesize every Frame of the output.
	 *
	 *  \authors    Maria Astrinaki, Alexis Moinet, Geoffrey Wilfart, Nicolas d'Alessandro, Thierry Dutoit
	 *
	 *  \version	2.00
	 *  \date		2011 - 2012
	 *  \copyright 
	 *				Numediart Institute for New Media Art ( www.numediart.org )	\n
	 *				Acapela Group ( www.acapela-group.com )						\n
	 *				GNU Public License (see the licence in the file).
	 */	
	class Vocoder 
	{
		public:
		
			/**
			 *	Constructor that allocates and initializes all the parameters needed for the Vocoder.
			 */
			Vocoder( int am = ( nOfMGCs-1 ),		double aalpha = defaultAlpha, 
					 int afprd = defaultFrameRate,	int aiprd = defaultInterpFrameRate, 
					 int astage = defaultGamma,		int apd = defaultPadeOrder, 
					 bool angain = false );
		
			/**
			 *	Constructor that allocates and initializes all the parameters needed for the Vocoder.
			 */
			Vocoder( const Vocoder& orig );
		
			/**
			 *	Destructor that disallocates all the memory used from the Vocoder.
			 */		
			virtual ~Vocoder();
			
// getters
		
			/**
			 *	This function gets the current value of the alpha parameter used in the Vocoder.
			 *
			 *	@return The alpha parameter value used in the Vocoder.
			 */
			inline double getAlpha ( void ){ return( this->alpha ); };
		
			/**
			 *	This function gets the current value of the pitch parameter used in the Vocoder.
			 *
			 *	@return The pitch parameter value used in the Vocoder.
			 */
			inline double getPitch ( void ){ return( this->f0 ); };
		
			/**
			 *	This function gets the current value of the period parameter used in the Vocoder.
			 *
			 *	@return The period parameter value used in the Vocoder.
			 */
			inline double getPeriod( void ){ return( this->t0 ); };
		
			/**
			 *	This function gets the current value of the volume parameter used in the Vocoder.
			 *
			 *	@return The volume parameter value used in the Vocoder.
			 */
			inline double getVolume( void ){ return( this->volume ); };
		
			/**
			 *	This function gets the current value of the pade order parameter used in the Vocoder.
			 *
			 *	@return The pade order parameter value used in the Vocoder.
			 */
			inline double getPadeOrder( void ){ return( this->pd ); };
			
			/**
			 *	This function gets the current action over the pitch control used in the Vocoder.
			 *
			 *	@return The action over the pitch control used in the Vocoder (overwrite, shift, scale, etc.).
			 */		
			inline int getAction( void ){ return( this->action ); };
		
			/**
			 *	This function gets the current value of the gamma parameter used in the Vocoder.
			 *
			 *	@return The gamma parameter value used in the Vocoder.
			 */
			inline double getGamma ( void ){ return( this->gamma = this->stage ); };

// setters
		
			/**
			 *	This function sets a new value of the alpha parameter to be used in the Vocoder.
			 *
			 *	@param alpha The new value of the alpha parameter.
			 */
			inline void setAlpha ( double alpha ){ this->alpha  = alpha; };	
		
			/**
			 *	This function sets a new value of the gamma parameter to be used in the Vocoder.
			 *
			 *	@param gamma The new value of the gamma parameter.
			 */
			inline void setGamma ( double gamma ){ this->stage  = gamma; };	
		
			/**
			 *	This function sets a new value of the volume parameter to be used in the Vocoder.
			 *
			 *	@param volume The new value of the volume parameter.
			 */
			inline void setVolume( double volume ){ this->volume = volume; };	
		
			/**
			 *	This function sets a new value of the pade order parameter to be used in the Vocoder.
			 *
			 *	@param padeOrder The new value of the pade order parameter.
			 */
			inline void setPadeOrder( double padeOrder ){ this->pd = padeOrder; };

			/**
			 *	This function forces a Frame to be voiced or unvoiced.
			 *
			 *	@param forceVoiced The new volume of the voiced / unvoiced flag.
			 */
			void setVoiced( bool forceVoiced );
		
			/** 
			 *	This function forces the value of the pitch used by the Vocoder instead of the one in Frame (f0).
			 *
			 *  \brief	Note that this will get overwritten at the next push( Frame ). Therefore it is needed to 
			 *			call setPitch() after every push(). Another solution is to call push( Frame, true ) which
			 *			explicitely tells push to ignore voicing information.
			 *
			 *	@param pitch The pitch value in Hz.
			 *	@param action The action over the pitch control (overwrite, shift, scale, etc.).
			 *	@param forceVoiced In case the current Frame is unvoiced, you can force it to become voiced with
			 *						the given pitch otherwise it would ignore the pitch set until next Frame.
			 */
			void setPitch( double pitch, int action, bool forceVoiced=false );
		
// methods
		
			/** 
			 *	This function receives a Frame and prepare all the parameters of the MLSA filter for the next sample generation by pop().
			 * 
			 *	@param frame An instance of class Frame.
			 *	@param ignoreVoicing If true, then ignore the voiced / unvoiced information of the Frame and use latest known information.
			 */
			void push( Frame &frame, bool ignoreVoicing=false );
		
			/**
			 *	This function receives a Frame and prepare all the parameters of the MLSA filter for the next sample generation by pop().
			 * 
			 *	@param frame A pointer to an instance of class Frame.
			 *	@param ignoreVoicing If true, then ignore the voiced / unvoiced information of the Frame and use latest known information.
			 */
			void push( Frame * frame, bool ignoreVoicing=false );
		
			/** 
			 *	This function resets the Vocoder to its default values.
			 *
			 *	\brief It sets the internal members of the Vocoder class to the constructor (defaults) values.
			 *			To be used in case the Vocoder becomes irremediably unstable or to reset all the changes.
			 */		
			void reset( void );

			/** 
			 *	This function returns true if at least one Frame has been pushed, otherwise false.
			 * 
			 *	@return True if at least one Frame has been pushed, otherwise false.
			 */
			bool ready( void );
		
			/** 
			 *	This function returns a single sample from the Vocoder.
			 * 
			 *	@return One sample from the Vocoder given current spectral coefficients and fundamental frequency.
			 */
			double pop( void );

// accessors

			/** 
			 *	This function returns true if the current Frame is voiced and false otherwise.
			 * 
			 *	@return True if the current Frame is voiced and false otherwise.
			 */
			bool isVoiced( void );
		
		protected:
			
			/** 
			 *	\var double alpha.
			 *	\brief It contains the alpha parameter used by the Vocoder.
			 */
			double alpha; // [0. 1]
		
			/** 
			 *	\var double gamma.
			 *	\brief It contains the gamma parameter used by the Vocoder.
			 */
			double gamma;
		
// excitation
		
			/** 
			 *	\var int action.
			 *	\brief It contains the action over the pitch control (overwrite, shift, scale, etc.).
			 */
			int action;
		
			/** 
			 *	\var bool voiced.
			 *	\brief It contains the voiced / unvoiced information of the Frame.
			 */
			bool voiced;
		
			/** 
			 *	\var double f0.
			 *	\brief It contains the current pitch (Hz) used by the Vocoder.
			 */
			double f0;
		
			/** 
			 *	\var double t0.
			 *	\brief It contains the period used by the Vocoder.
			 */
			double t0;
		
			/** 
			 *	\var double volume.
			 *	\brief It contains the volume used by the Vocoder.
			 */
			double volume; // >= 0

		private:
		
			int m;
			int fprd;
			int iprd;
			int stage;
			int pd;
			int csize;
		
			bool ngain;
			bool flagFirstPush;
		
			double x;
			double * c;
			double * inc;
			double * cc;
			double * d;
				
			double padesptk[21];
			double * ppadesptk;
		
			int count;
			double actionValue;
		
			int nOfPopSinceLastPush;
		
			// functions imported from SPTK
			void mc2b( double * mc, double * b, int m, const double a );
			void gnorm( double * c1, double * c2, int m, const double g );
			void movem( void * a, void * b, const size_t size, const int nitem );

			double mglsadf  ( double x, double * b, const int m, const double a, const int n, double * d );
			double mglsadff ( double x, double * b, const int m, const double a, double * d );
			double mglsadft ( double x, double * b, const int m, const double a, const int n, double * d );
			double mglsadfft( double x, double * b, const int m, const double a, double * d );

			double mlsadf ( double x, double * b, const int m, const double a, const int pd, double * d );
			double mlsafir( double x, double * b, const int m, const double a, double * d );
			double mlsadf1( double x, double * b, const int m, const double a, const int pd, double * d );
			double mlsadf2( double x, double * b, const int m, const double a, const int pd, double * d );
	};	
} // namespace


