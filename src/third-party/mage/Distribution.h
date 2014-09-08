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

namespace MAGE 
{
	/** 
	 *  \brief		Definition of a gaussian distribution.
	 *  \details	This struct is used to define a gaussian distribution given the mean and variance as parameters.
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
	struct Distribution 
	{
		/** 
		 *	\var double mean.
		 *	\brief It contains the mean value of a gaussian distribution.
		 */
		double mean;
		
		/** 
		 *	\var double vari.
		 *	\brief It contains the variance value of a gaussian distribution.
		 */
		double vari;
		
		/** 
		 *	\var double msdFlag.
		 *	\brief It contains the MSD value of a gaussian distribution.
		 */
		double msdFlag;
	};
} // namespace
