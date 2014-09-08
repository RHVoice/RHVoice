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

#include "Distribution.h"
#include "Constants.h"

namespace MAGE 
{
	/** 
	 *  \brief		Definition of the HMMs.
	 *  \details	This struct is used to define every state of the HMMs used. Here it contains the duration and distributions used in every state of HMM.
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
	struct State 
	{
		/** 
		 *	\var int duration.
		 *	\brief It contains the duration of every state of a given HMM.
		 */
		int duration;
	
// global variances switch

		/** 
		 *	\var bool gv_switch_streams.
		 *	\brief It contains the global variance flag of the spectral coefficients stream, 
		 *			the fundamental frequency coefficient stream and the low-pass 
		 *			filter coefficients stream for every state of a given HMM.
		 */
		bool gv_switch_streams[nOfStreams];
	
// parameters loaded from trees
		
		/** 
		 *	\var Distribution streams.
		 *	\brief It contains the gaussian distribution the spectral coefficients stream, 
		 *			the fundamental frequency coefficient stream and the low-pass 
		 *			filter coefficients stream (including static and dynamic features) 
		 *			for every state of a given HMM.
		 */
		Distribution streams[nOfStreams][maxStreamLen];
		
// global variances
		
		/** 
		 *	\var Distribution gv_streams.
		 *	\brief It contains the gaussian distribution of the global variance of 
		 *			the spectral coefficients stream, the fundamental frequency 
		 *			coefficient stream and the low-pass filter coefficients stream 
		 *			(including static and dynamic features) for every state of a given HMM.
		 */
		Distribution gv_streams[nOfStreams][maxStreamLen];
		
	};
} // namespace
