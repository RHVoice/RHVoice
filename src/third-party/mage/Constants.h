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

#define HTS 0 // HTS 

// #define mode HTS 

// #if mode == HTS
#if 1
	#define Alpha 0.466
	#define PadeOrder 5
	#define NumberOfStreams 3 

	#define logF0 1
	#define melF0 0
	#define NumberOfMGCs 31		// # of MGC coefficients for the MLSA filter	
#endif

namespace MAGE 
{

// --- Parameter Length ---

	/**
	 *	\var const unsigned int nOfDers.
	 *	\brief Number of derivations: ∆( 0 ), ∆( 1 ), ∆( 3 ).
	 *
	 */
	const unsigned int nOfDers = 3;		// # of derivations: ∆( 0 ), ∆( 1 ), ∆( 3 )
	
	/**
	 *	\var const unsigned int nOfMGCs.
	 *	\brief Number of spectral coefficients for the MLSA filter.
	 *
	 */
	const unsigned int nOfMGCs = NumberOfMGCs;	// # of MGC coefficients for the MLSA filter
	
	/**
	 *	\var const int mgcLen.
	 *	\brief Number of spectracl coefficients including including static and dynamic features.
	 *
	 */		
	const int mgcLen = nOfDers * nOfMGCs;
	
	/**
	 *	\var const unsigned int nOfLF0s.
	 *	\brief Number of fundamental frequency coefficients (here it is a single value).
	 *
	 */
	const unsigned int nOfLF0s = 1;		// fundamental frequency is a single value
	
	/**
	 *	\var const int mgcLen.
	 *	\brief Number of fundamental frequency coefficients including including static and dynamic features.
	 *
	 */	
	const int lf0Len = nOfDers * nOfLF0s;	
	
	/**
	 *	\var const unsigned int nOfBAPs.
	 *	\brief Number of band aperiodicities.
	 *
	 */
	const unsigned int nOfBAPs = 12;	// # of band aperiodicities
	
	/**
	 *	\var const int mgcLen.
	 *	\brief Number of band aperiodicities including including static and dynamic features.
	 *
	 */	
	const int bapLen = nOfDers * nOfBAPs;
	
	const int maxStreamLen = nOfDers * nOfMGCs;

	/**
	 *	\var const int maxWindowWidth.
	 *	\brief Maximum number for the coefficients of a window.
	 *
	 */	
	const int maxWindowWidth = 50;
	
	/**
	 *	\var const unsigned int nOfStates.
	 *	\brief Number of states in the HMM.
	 *
	 */
	const unsigned int nOfStates  = 5;	// # of states in the HMM
	
	/**
	 *	\var const unsigned int nOfStreams.
	 *	\brief Number of streams, here mgcs, lf0, bap.
	 *
	 */
	const unsigned int nOfStreams = NumberOfStreams;	// # of streams : mgcs, lf0, bap
	
	
// --- Queues ---
	
	/**
	 *	\var const unsigned int nOfLookup.
	 *	\brief Number of looked-up Model instances (labels), not used yet that 
	 *			we keep in memory for smoother parameters computation.
	 *
	 */
	//const unsigned int nOfLookup = 1;	// # of looked-up labels
	const unsigned int nOfLookup = 0;	// # of looked-up labels
	
	/**
	 *	\var const unsigned int nOfBackup.
	 *	\brief Number of backed-up Model instances (labels), already used that 
	 *			we keep in memory for smoother parameters computation.
	 *
	 */
	const unsigned int nOfBackup = 2;	
	// # of backed-up labels( models in modelqueue actually )
	// this is the number of labels/models already used that 
	// we keep in memory for smoother parameters computation
	
	
	/**
	 *	\var const int maxNumOfFrames.
	 *	\brief Maximum number of frames per phoneme.
	 *
	 */	
	const int maxNumOfFrames = 512;		// maximum # of frames per phoneme
	
	/**
	 *	\var const int maxDuration.
	 *	\brief Maximum number of frames per model since ModelQueueMemory has only maxNumOfFrames allocated.
	 *
	 */
	const int maxDuration = ( int ) maxNumOfFrames / ( nOfBackup + nOfLookup + 1 ); 
	//	fix for memory overflow in optimizeParameters.
	//	ModelQueueMemory has only maxNumOfFrames allocated) 
	//	which means 'maxNumOfFrames/(nofLookup+nofBackup+1)'
	//	frames per model.
	
	/**
	 *	\var const int maxLabelQueueLen.
	 *	\brief Maximum number of Label inctances (labels) that can be pushed in the LabelQueue.
	 *
	 */
	const int maxLabelQueueLen = 512; // max amount of labels that can wait
	
	/**
	 *	\var const int maxFrameQueueLen.
	 *	\brief Maximum number of Frame inctances that can be pushed in the FrameQueue
	 *			( longest Label inctance 1 sec = 200 frames of 5 smsec ). 
	 *
	 */
	const int maxFrameQueueLen = 200; // longest label 1 sec = 200 frames of 5 smsec
	
	/**
	 *	\var const int maxModelQueueLen.
	 *	\brief Maximum number of stored past and future Model instances for the parameter generation / optimization.
	 *
	 */
	const int maxModelQueueLen = nOfLookup + nOfBackup + 2; // max stored past and future models for generation
	
// --- Vocoder ---
	
	/**
	 *	\var const double defaultAlpha.
	 *	\brief Default value for the alpha parameter used also during the training phase.
	 *
	 */
	const double defaultAlpha = Alpha;
	
	/**
	 *	\var const int defaultFrameRate.
	 *	\brief Default value for the framerate parameter used also during the training phase.
	 *
	 */
	const int defaultFrameRate = 120;
	
	/**
	 *	\var const int defaultInterpFrameRate.
	 *	\brief Default value for the interpolation framerate parameter used also during the training phase.
	 *
	 */
	const int defaultInterpFrameRate = 1;
	
	/**
	 *	\var const int defaultSamplingRate.
	 *	\brief Default value for the sampling rate parameter used also during the training phase.
	 *
	 */
	const int defaultSamplingRate = 24000;
	
	const int fftLen = 256;
	
	/**
	 *	\var const int defaultPadeOrder.
	 *	\brief Default value for the pade order parameter used also during the training phase.
	 *
	 */
	const int defaultPadeOrder = PadeOrder;

#if PadeOrder == 5
	const double R1 = 6.0;
	const double R2 = 7.65;
#endif

#if PadeOrder == 4
	const double R1 = 4.5;
	const double R2 = 6.2;
#endif	
	
	/**
	 *	\var const int defaultGamma.
	 *	\brief Default value for the gamma parameter used also during the training phase.
	 *
	 */
	const int defaultGamma = 0;
	
	/**
	 *	\var const int defaultVolume.
	 *	\brief Default value for the volume parameter used also during the training phase.
	 *
	 */	
	const int defaultVolume = 1;
	
	/**
	 *	\var const int defaultPitch.
	 *	\brief Default value for the pitch parameter.
	 *
	 */	
	const int defaultPitch = 110; // Hz

#if logF0 == 1
	const int useLF0 = 1;
	const int useMELF0 = 0;
#endif
	
#if melF0 == 1
	const int useLF0 = 0;
	const int useMELF0 = 1;
#endif
	
	const bool enableMLSAcheck = true; 

// --- Stream Index --- 

#if NumberOfStreams == 2
	/**
	 *	\var const int mgcStreamIndex.
	 *	\brief Index number for the spectracl coefficients stream.
	 *
	 */
	const int mgcStreamIndex = 0; // mgc stream index
	
	/**
	 *	\var const int lf0StreamIndex.
	 *	\brief Index number for the fundamental frequency stream.
	 *
	 */
	const int lf0StreamIndex = 1; // lf0 stream index
	
	/**
	 *	\var const int durStreamIndex.
	 *	\brief Index number for the State duration stream.
	 *
	 */
	const int durStreamIndex = 2; // duration stream index
#endif

	
#if NumberOfStreams == 3
	/**
	 *	\var const int mgcStreamIndex.
	 *	\brief Index number for the spectracl coefficients stream.
	 *
	 */
	const int mgcStreamIndex = 0; // mgc stream index
	
	/**
	 *	\var const int lf0StreamIndex.
	 *	\brief Index number for the fundamental frequency stream.
	 *
	 */
	const int lf0StreamIndex = 1; // lf0 stream index
	
	/**
	 *	\var const int bapStreamIndex.
	 *	\brief Index number for the band aperiodicities stream.
	 *
	 */
	const int bapStreamIndex = 2; // bap stream index
	
	/**
	 *	\var const int durStreamIndex.
	 *	\brief Index number for the State duration stream.
	 *
	 */
	const int durStreamIndex = 3; // duration stream index
#endif
	
// --- Actions ---

	/**
	 *	\var const int noaction.
	 *	\brief Flag number for taking no action over a variable.
	 *
	 */
	const int noaction = -1;
	
	/**
	 *	\var const int overwrite.
	 *	\brief Flag number for replacing / overwriting a variable.
	 *
	 */
	const int overwrite = 0;
	
	/**
	 *	\var const int shift.
	 *	\brief Flag number for shifting a variable (+ the given shift value).
	 *
	 */
	const int shift = 1;
	
	/**
	 *	\var const int scale.
	 *	\brief Flag number for scaling a variable (* the given scale value).
	 *
	 */
	const int scale	= 2;
	
	/**
	 *	\var const int synthetic.
	 *	\brief Flag number for keeping the synthetic variable generated by the model.
	 *
	 */
	const int synthetic	= 3;
	
// --- Configuration files ---
	
	/**
	 *	\var const int maxNumOfArguments.
	 *	\brief Maximum number of arguments passed by the configuration file.
	 *
	 */
	const int maxNumOfArguments = 100;
	
// --- Max string lenght ---
	
	/**
	 *	\var const int maxStrLen.
	 *	\brief Maximum number of characters in a string.
	 *
	 */
	const int maxStrLen = 1024;
	
// --- Interpolation Wheight ---
	
	/**
	 *	\var const double defaultInterpolationWeight.
	 *	\brief Default interpolation weight of a given Model.
	 *
	 */
	const double defaultInterpolationWeight = 1;
	
	const double defaultMSDflag = 0;

} // namespace	

