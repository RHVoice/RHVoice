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

#include "Constants.h"
#include "State.h"
#include "Label.h"
#include "hts.h"

#include <vector>
#include <cstring>

namespace MAGE 
{
	/** 
	 *  \brief		The memory used for every Model.
	 *  \details	This class is used to define the memory structures that are needed for a single Model.
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
	class ModelMemory 
	{
		public :
		
			/**
			 *	Constructor that allocates the required memory for a single Model.
			 */
			ModelMemory();
		
			/**
			 *	Destructor that disallocates all the memory used from a single Model.
			 */	
			~ModelMemory();
				
			/** 
			 *	\var double * duration_mean.
			 *	\brief It contains the mean value of the duration for every State of a given HMM.
			 */
			double * duration_mean; 

			/** 
			 *	\var double * duration_vari.
			 *	\brief It contains the variance value of the duration for every State of a given HMM.
			 */
			double * duration_vari;
		
			/** 
			 *	\var double * duration_array.
			 *	\brief It contains the duration for every State of a given HMM.
			 */
			int * duration_array;
		
// # of MGC coefficients for the MLSA filter & # of derivations: ∆( 0 ), ∆( 1 ), ∆( 3 )
// fundamental frequency & # of derivations: ∆( 0 ), ∆( 1 ), ∆( 3 )
// # of low-pass filter coefficients & # of derivations: ∆( 0 ), ∆( 1 ), ∆( 3 )

			/** 
			 *	\var double * stream_mean.
			 *	\brief It contains the mean value of the spectral coefficients stream, 
			 *			the fundamental frequency coefficient stream and the low-pass 
			 *			filter coefficients stream (including static and dynamic features) 
			 *			for every State of a given HMM.
			 */
			double ** stream_mean; // [nOfStreams][maxStreamLen]
		
			/** 
			 *	\var double * stream_vari.
			 *	\brief It contains the variance value of the spectral coefficients stream, 
			 *			the fundamental frequency coefficient stream and the low-pass 
			 *			filter coefficients stream (including static and dynamic features) 
			 *			for every State of a given HMM.
			 */
			double ** stream_vari; // [nOfStreams][maxStreamLen]
			
			/** 
			 *	\var char strQuery.
			 *	\brief It contains the string query to retrieve the coefficients for every stream (mean & variance) for every 
			 *			State of the Model.
			 */
			char strQuery[maxStrLen];
	};

	/** 
	 *  \brief     The HMM Model used.
	 *  \details   This class is used to define the Model structures that are needed for an HMM Model.
	 *
	 *  \authors    Maria Astrinaki, Alexis Moinet, Geoffrey Wilfart, Nicolas d'Alessandro, Thierry Dutoit
	 *
	 *  \version   2.00
	 *  \date      2011 - 2012
	 *  \copyright 
	 *				Numediart Institute for New Media Art ( www.numediart.org )	\n
	 *				Acapela Group ( www.acapela-group.com )						\n
	 *				GNU Public License (see the licence in the file).
	 */	
	class Model 
	{	
		public :
		
			/**
			 *	Constructor that allocates the required memory for a Model and initializes the parameters used.
			 */
			Model();
		
			/**
			 *	Destructor that disallocates all the memory used from a Model.
			 */
			~Model();
		
// getters

			/**
			 *	This function gets a Model State.
			 *
			 *	@param index Number / index of the State to be returned.
			 *	@return A Model State given an index.
			 */
			const State& getState( int index ) const;
          State& getState( int index );
		
			/**
			 *	This function gets the total duration of the Model.
			 *
			 *	@return The total duration of the Model.
			 */
			int getDuration( void );

//setters
		
			/**
			 *	This function sets a Model State.
			 *
			 *	@param state The State to be set.
			 *	@param index Number / index of the State to be set.
			 */
			void setState( State state, int index );
		
			/**
			 *	This function sets the total duration of the Model.
			 *
			 *	@param duration The total duration to be set for the Model.
			 */
			void setDuration( int duration );
		
// methods
		
			/**
			 *	This function initializes the duration of every State of the Model to zero.
			 *
			 */
			void initDuration  ( void );
			
			/**
			 *	This function initializes the mean and variance of every coefficients stream to zero.
			 *
			 */
			void initParameters( void );
		
			/**
			 *	This function changes the duration of every State of the Model.
			 *
			 *	@param updateFunction The new durations to be passed.
			 *	@param action The action that will be taken between the existing and the passed durations ( overwritten, shifted or scaled ).
			 */
			void updateDuration( double * updateFunction, int action ); // to put a speed profile on State duration( put it inside compute duration ? )
		
			/**
			 *	This function computes the duration of every State of the Model.
			 *
			 *	@param engine The engine to be used.
			 *	@param label The string query for which the durations are going to be computed.
			 *	@param interpolationWeight The possible interpolation weights to be taken into account for the computation.
			 *			If this argument is set to NULL then by default the interpolation weights taken into account are set to 1.
			 */
			void computeDuration  ( MAGE::Engine * engine, MAGE::Label * label, double * interpolationWeight );
		
			/**
			 *	This function computes the parameters for every coefficients stream of every State of the Model.
			 *
			 *	@param engine The engine to be used.
			 *	@param label The string query for which the parameters are going to be computed.
			 *	@param interpolationWeight The possible interpolation weights to be taken into account for the computation.
			 *			If this argument is set to NULL then by default the interpolation weights taken into account are set 
			 *			to one as default value.
			 */
			void computeParameters( MAGE::Engine * engine, MAGE::Label * label, double * interpolationWeight );
		
			/**
			 *	This function computes the global variances for every coefficients stream of every State of the Model.
			 *
			 *	@param engine The engine to be used.
			 *	@param label The string query for which the global variances are going to be computed.
			 */
			void computeGlobalVariances( MAGE::Engine * engine, MAGE::Label * label );
		
			/**
			 *	This function checks and normalizes the interpolation weights for every coefficients stream of every State of  
			 *		the Model that are passed directly to the given engine from the configuration file.
			 *
			 *	@param engine The engine to be used.
			 *	@param forced The flag to recall this function several times.
			 */
			void checkInterpolationWeights( MAGE::Engine * engine, bool forced = false );

		protected :
		
			/** 
			 *	\var int duration.
			 *	\brief It contains the total duration of the Model.
			 */
			int duration;
		
			/** 
			 *	\var State state.
			 *	\brief It contains the total number of states discribing the Model.
			 */
			State state[nOfStates];
		
			/** 
			 *	\var ModelMemory modelMemory.
			 *	\brief It contains the total amount used by the Model.
			 */
			ModelMemory modelMemory;
		
		private :
		
			// The string query for which all the coefficients are going to be computed.
			char strQuery[maxStrLen];
		
			// Flag checking if the weights have been already checked.
			bool weightsChecked;
	};
} // namespace
