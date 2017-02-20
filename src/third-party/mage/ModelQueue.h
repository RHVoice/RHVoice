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

#include "mlsacheck-sptk.h"

#include "MemQueue.h"
#include "Model.h"

#include "FrameQueue.h"
#include "Frame.h"

#include "MathFunctions.h"

namespace MAGE 
{	
	/** 
	 *  \brief		The memory used by a ModelQueue used in Mage.
	 *  \details	This class is used to define the memory structures that are needed for a ModelQueue.
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
	class ModelQueueMemory 
	{
		public :
		
			/**
			 *	Constructor that allocates the required memory for a Model queue.
			 */
			ModelQueueMemory();
			
			/**
			 *	Destructor that disallocates all the memory used from a Model queue.
			 */	
			~ModelQueueMemory();
		
			/** 
			 *	\var double mean.
			 *	\brief It contains the mean vector sequence of every stream, of every Frame.
			 */
			double *** mean;	// for every stream, for every frame, every mean
								// [nOfStreams][maxNumOfFrames][nOfDers * nOfMGCs]
		
			/** 
			 *	\var double ivar.
			 *	\brief It contains the inverse diag variance sequence of every stream, of every Frame.
			 */
			double *** ivar;	// for every stream, for every frame, every ivar
								// [nOfStreams][maxNumOfFrames][nOfDers * nOfMGCs] 
			
			/** 
			 *	\var double gv_mean.
			 *	\brief It contains the global variance mean vector sequence of every stream.
			 */
			double ** gv_mean;	// for every stream, every gv_mean
								// [nOfStreams][maxNumOfFrames][nOfDers * nOfMGCs] 
		
			/** 
			 *	\var double gv_vari.
			 *	\brief It contains the global variance variance vector sequence of every stream.
			 */
			double ** gv_vari;	// for every stream, every gv_var
								// [nOfStreams][maxNumOfFrames][nOfDers * nOfMGCs] 
			
			/** 
			 *	\var int gv_switch.
			 *	\brief It contains the global variance flag sequence of every stream.
			 */
			int ** gv_switch;	// for every stream, every gv_switch
								// [nOfStreams][maxNumOfFrames][nOfDers * nOfMGCs] 
		
			/** 
			 *	\var double g.
			 *	\brief It contains the vector used in the forward substitution of every stream.
			 */
			double ** g;		// for every stream	// [nOfStreams][maxNumOfFrames];
		
			/** 
			 *	\var double wum.
			 *	\brief It contains the " W' U^-1 mu " vector of every stream.
			 */
			double ** wum;		// for every stream	// [nOfStreams][maxNumOfFrames];
		
			/** 
			 *	\var double wuw.
			 *	\brief It contains the " W' U^-1 W " vector of every stream.
			 */
			double *** wuw;		// for every stream	// [nOfStreams][maxNumOfFrames][maxWindowWidth]
		
			/** 
			 *	\var double par.
			 *	\brief It contains the output parameter vector for the otimized spectral coefficients, 
			 *			fundamental frequency and low-pass filter coefficients of every stream 
			 *			(including static and dynamic features).
			 */
			double *** par;		// output parameter vector for otimized mgc, lf0 and bap for every stream, 
								// for every frame // [nOfStreams][maxNumOfFrames][nOfDers * nOfMGCs] 
		
			/** 
			 *	\var int voiced_unvoiced.
			 *	\brief It contains the voiced / unvoiced flag sequence of every Frame.
			 */		
			int * voiced_unvoiced; // [maxNumOfFrames]		
	};
	
	/** 
	 *  \brief		The memory queue of Model instances used in Mage (Model ringbuffer).
	 *  \details	This class is used to exchange the Model instances between the different threads; 
	 *				it stores statistical models and the special generate() function takes a short
	 *				lookup window and generates oldest-label frames.
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
	class ModelQueue : public MemQueue<Model> 
	{
		public:
		
			/**
			 *	Constructor that allocates the required memory for a ModelQueue.
			 * 
			 *	@param queueLen The max size of the ModelQueue. i.e. how many Model instances the ringbuffer 
			 *					can contain before it's full.
			 */
			ModelQueue( unsigned int queueLen );
		
			/**
			 *	Destructor that disallocates all the memory used from a ModelQueue.
			 */	
			~ModelQueue();
			
// getters 
		
			/**
			 *	This function gets the current memory used by the ModelQueue.
			 *
			 *	@return The memory (as a ModelQueueMemory) used by the ModelQueue.
			 */
			inline ModelQueueMemory *getModelQueueMemory( void ){ return( &this->modelQueueMemory ); };

// methods
		
			/**
			 *	This function generates the PDF parameters of every stream.
			 *
			 *	@param engine The Engine used to store the Model and Global parameter structures.
			 *	@param frameQueue The FrameQueue used to store the generated parameters of every Frame.
			 *	@param backup The number of Model instances already used that we keep in memory 
			 *			for computation of smoother parameters.
			 */
			void generate( MAGE::Engine * engine, FrameQueue * frameQueue, unsigned int backup = nOfBackup );
		
			/**
			 *	This function optimizes the generated parameters of every stream.
			 *
			 *	@param engine The Engine used to store the Model and Global parameter structures.
			 *	@param backup The number of Model instances already used that we keep in memory 
			 *			for computation of smoother parameters.
			 *	@param lookup The number of Model instances not yet used that we keep in memory 
			 *			for computation of smoother parameters. Ideally should be 0 for better reactivity.
			 */
			void optimizeParameters( MAGE::Engine * engine, unsigned int backup = nOfBackup, unsigned int lookup = nOfLookup );
		
			/**
			 *	This function prints the content of a ModelQueue.
			 */
			void printQueue( void );

		protected:

			/**
			 * \var Frame * frame
			 * \brief pointer to an item in a FrameQueue. Used internally only for now, could be changed in local variable
			 */
			Frame * frame;

			/**
			 * \var unsigned int * head
			 * \brief pointer to actual head of the ModelQueue, taking into account the backup models. Used internally only for now, could be changed in local variable, but was used across several functions at some point.
			 */
			unsigned int head;
			
			/**
			 * \var ModelQueueMemory * modelQueueMemory
			 * \brief constant-sized memory used to store all the models. Allocated once at the beginning of Mage, when ModelQueue constructor is called, to avoid any on-the-fly allocation
			 */
			ModelQueueMemory modelQueueMemory;
	};	
} // namespace
