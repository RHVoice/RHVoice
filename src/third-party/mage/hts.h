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

#pragma once

#include "MathFunctions.h"
#include "HTS106_engine.h"
#include "HTS106_hidden.h"

#include <string>
#include <stdarg.h>
#include <cstdlib>

using namespace std;

namespace MAGE 
{
	/** 
	 *  \brief		The default HTS Engine used for HMM-based Speech Synthesis as provided by http://hts.sp.nitech.ac.jp/ .
	 *  \details	This class is used to define the default Engine instance to be used in Mage (here HTS Engine).
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
	
	class Engine
	{
		public:
		
			/**
			 *	Constructor that allocates the required memory for an Engine instance and initializes 
			 *	all the parameters in default values.
			 */	
			Engine();
		
			/**
			 *	Destructor that disallocates all the memory used from an Engine instance.
			 */	
			~Engine();
		
// getters
		
			/**
			 *	This function gets the global settings used by an Engine instance.
			 *
			 *	@return The global settings used by an Engine instance.
			 */
			inline HTS106_Global getGlobal( void ){ return( this->global ); };
		
			/**
			 *	This function gets an individual PDF stream used by an Engine instance.
			 *
			 *	@return The individual PDF stream used by an Engine instance.
			 */
			inline HTS106_PStream getPStream( void ){ return( this->pss ); };
		
			/**
			 *	This function gets the set of duration models, HMMs and global variance models used by an Engine instance.
			 *
			 *	@return The set of duration models, HMMs and global variance models used by an Engine instance.
			 */
			inline HTS106_ModelSet getModelSet( void ){ return( this->ms ); };

//setters
			/**
			 *	This function sets new global settings to the Engine instance.
			 *
			 *	@param global The new global settings for the Engine instance.
			 */	
			inline void setGlobal( HTS106_Global global ){ this->global = global; };
		
			/**
			 *	This function sets a new PDF stream used by an Engine instance.
			 *
			 *	@param pss The new PDF stream for the Engine instance.
			 */
			inline void setPStream( HTS106_PStream pss ){ this->pss = pss; };
		
			/**
			 *	This function sets a new set of duration models, HMMs and global variance models 
			 *	used by an Engine instance.
			 *
			 *	@param ms The new set of duration models, HMMs and global variance models
			 *					for the Engine instance.
			 */
			inline void setModelSet( HTS106_ModelSet ms ){ this->ms = ms; };

// methods
		
			/**
			 *	This function initializes all the variables needed for an Engine instance.
			 *
			 *	@param argc The number of arguments passed.
			 *	@param argv The actual arguments passed to be used for the initialization.
			 */
			void load( int argc, char ** argv );
		
	protected:
		
			/** 
			 *	\var HTS106_Global global.
			 *	\brief It contains the global settings used from an Engine instance.
			 */
			HTS106_Global global;	// global settings 
		
			/** 
			 *	\var HTS106_PStream pss.
			 *	\brief It contains an individual PDF stream used from an Engine instance.
			 */
			HTS106_PStream pss;	// set of PDF streams
		
			/** 
			 *	\var HTS106_ModelSet ms.
			 *	\brief It contains a set of duration models, HMMs and GV models
			 *			used from an Engine instance.
			 */
			HTS106_ModelSet ms;	// set of duration models, HMMs and GV models 

		
	private:
		
		HTS106_Engine engine;
		
		 /* delta window handler for mel-cepstrum*/
		char ** fn_ws_mgc;
		
		 /* delta window handler for log f0*/
		char ** fn_ws_lf0;
		
		 /* delta window handler for low-pass filter*/
		char ** fn_ws_bap;
		
		 /* prepare for interpolation*/
		double * rate_interp;
		
		char ** fn_ms_dur;
		char ** fn_ms_mgc;
		char ** fn_ms_lf0;
		char ** fn_ms_bap;
		char ** fn_ts_dur;
		char ** fn_ts_mgc;
		char ** fn_ts_lf0;
		char ** fn_ts_bap;
		char ** fn_ms_gvm;
		char ** fn_ms_gvl;
		char ** fn_ms_gvf;
		char ** fn_ts_gvm;
		char ** fn_ts_gvl;
		char ** fn_ts_gvf;
		char * fn_gv_switch;
		
	};
}// namespace

// --- HTS Engine ---

/**
 *	This function prints the output usage for the initialization of an Engine instance.
 *
 *	\see http://hts.sp.nitech.ac.jp/
 */
void Usage( void );

/**
 *	This function prints an output error message.
 *
 *	\see http://hts.sp.nitech.ac.jp/
 *
 *	@param error The number of the error.
 *	@param message The message of the passed error.
 */
void Error( const int error, char * message, ... );

/**
 *	This function gets the number of speakers for interpolation from the arguments passed 
 *	for the initialization of an Engine instance.
 *
 *	\see http://hts.sp.nitech.ac.jp/
 *
 *	@param argc The number of the passed arguments.
 *	@param argv_search The actual arguments passed.
 */
int GetNumInterp( int argc, char ** argv_search );

// -- HTS Labels --

/**
 *	This function returns true if the passed string argument is representing 
 *	an integer number, and it returns false otherwise.
 *
 *	\see http://hts.sp.nitech.ac.jp/
 *
 *	@param str The string to check if it contains an integer number.
 */
bool isdigit_string( char * str );

/**
 *	This function sets / retrieves the duration vector sequence from the 
 *	state duration probability distribution.
 *
 *	\see http://hts.sp.nitech.ac.jp/
 *
 *	@param duration The retrieved duration vector sequence.
 *	@param mean The mean of the state duration probability distribution.
 *	@param vari The variance of the state duration probability distribution.
 *	@param size The number of states that the HMM contains.
 *	@param frame_length The length of the given frame.
 */
double mHTS106_set_duration( int * duration, double * mean, double * vari, int size, double frame_length );

// --- HTS PStream --

/**
 *	This function calculates the inverse variance ( 1.0/variance ).
 *
 *	\see http://hts.sp.nitech.ac.jp/
 *
 *	@param x The variance to be converted.
 *	@return The converted inverse variance.
 */
double HTS106_finv( const double x );

/**
 *	This function generates the sequence of speech parameter vectors by maximizing its 
 *	output probability for a given pdf sequence.
 *
 *	\see http://hts.sp.nitech.ac.jp/
 *
 *	@param pst The individual PDF stream used from an Engine instance.
 */
void HTS106_PStream_mlpg( HTS106_PStream * pst );

/**
 *	This function calcurates the : W'U^{-1}W and W'U^{-1}M.
 *
 *	\see http://hts.sp.nitech.ac.jp/
 *
 *	@param pst The individual PDF stream used from an Engine instance.
 *	@param m The static legnth of the current PDF stream used from an Engine instance.
 */
void HTS106_PStream_calc_wuw_and_wum( HTS106_PStream * pst, const int m );

/**
 *	This function factorizes W' * U^{-1} * W to L * D * L'( L: lower triangular, D: diagonal ).
 *
 *	\see http://hts.sp.nitech.ac.jp/
 *
 *	@param pst The individual PDF stream used from an Engine instance.
 */
void HTS106_PStream_ldl_factorization( HTS106_PStream * pst );

/**
 *	This function computes the forward subtitution for mlpg.
 *
 *	\see http://hts.sp.nitech.ac.jp/
 *
 *	@param pst The individual PDF stream used from an Engine instance.
 */
void HTS106_PStream_forward_substitution( HTS106_PStream * pst );

/**
 *	This function computes the backward subtitution for mlpg.
 *
 *	\see http://hts.sp.nitech.ac.jp/
 *
 *	@param pst The individual PDF stream used from an Engine instance.
 *	@param m The static legnth of the current PDF stream used from an Engine instance.
 */
void HTS106_PStream_backward_substitution( HTS106_PStream * pst, const int m );


/**
 *	This function is used for the mlpg function using global variances.
 *
 *	\see http://hts.sp.nitech.ac.jp/
 *
 *	@param pst The individual PDF stream used from an Engine instance.
 *	@param m The static legnth of the current PDF stream used from an Engine instance.
 */
void HTS106_PStream_gv_parmgen( HTS106_PStream * pst, const int m );

/**
 *	This function is used as a subfunction for the mlpg function using global variances.
 *
 *	\see http://hts.sp.nitech.ac.jp/
 *
 *	@param pst The individual PDF stream used from an Engine instance.
 *	@param m The static legnth of the current PDF stream used from an Engine instance.
 */
void HTS106_PStream_conv_gv( HTS106_PStream * pst, const int m );

/**
 *	This function is used as a subfunction for the mlpg function using global variances.
 *
 *	\see http://hts.sp.nitech.ac.jp/
 *
 *	@param pst The individual PDF stream used from an Engine instance.
 *	@param m The static legnth of the current PDF stream used from an Engine instance.
 */
double HTS106_PStream_calc_derivative( HTS106_PStream * pst, const int m );

/**
 *	This function is used as a subfunction for the mlpg function using global variances.
 *
 *	\see http://hts.sp.nitech.ac.jp/
 *
 *	@param pst The individual PDF stream used from an Engine instance.
 *	@param m The static legnth of the current PDF stream used from an Engine instance.
 */
void HTS106_PStream_calc_gv( HTS106_PStream * pst, const int m, double * mean, double * vari );


