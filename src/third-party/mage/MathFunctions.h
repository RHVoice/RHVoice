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

#include <cstdlib>
#include <cmath>

namespace MAGE 
{
	/**
	 *	This function returns the maximum between two numbers.
	 *
	 *	@param a One of the two numbers to be compared.
	 *	@param b The other of the two numbers to be compared.
	 *	@return The maximum between two numbers.
	 */
	inline float Max( float a, float b )
	{
		return( ( a ) < ( b ) ? ( b ) : ( a ) );
	}

	/**
	 *	This function returns the minimum between two numbers.
	 *
	 *	@param a One of the two numbers to be compared.
	 *	@param b The other of the two numbers to be compared.
	 *	@return The minimum between two numbers.
	 */
	inline float Min( float a, float b )
	{
		return( ( a ) > ( b ) ? ( b ) : ( a ) );
	}

	/**
	 *	This function returns a random number between two numbers.
	 *
	 *	@param x The left limit for the random number to be computed.
	 *	@param y The right limit for the random number to be computed.
	 *	@return A random number between two given numbers.
	 */
	float Random( float x, float y );
	
	/**
	 *	This function returns the rounded inputed number.
	 *
	 *	@param x The number to be rounded.
	 *	@return The rounded inputed number.
	 */
	//	This function returns the rounded inputed number.
	double Round( double x );
	
} // namespace
