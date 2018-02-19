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

#include "Label.h"

//	Constructor that allocates the required memory for a single Label query.
MAGE::LabelMemory::LabelMemory()
{
}

//	Destructor that disallocates all the memory used from a single Label query.
MAGE::LabelMemory::~LabelMemory( void )
{
}

//	Constructor that initializes all the parameters needed for a single Label query in default values.
MAGE::Label::Label( void )
{
	this->query = "";
	this->isDurationForcedFlag = false;
	this->begin = this->end = -1;
	this->speed = 1.0;
}

//	Constructor that initializes all the parameters needed for a single Label query given a string query.
MAGE::Label::Label( string q )
{
	this->parseQuery( q );
	this->speed = 1.0;
}

//	Destructor that disallocates all the memory used from a Label.
MAGE::Label::~Label( void )
{
}

// getters


//setters
//	This function sets a new value of the query to be used in the Label.
void MAGE::Label::setQuery( string q )
{
	this->parseQuery( q );
	return;
}

// methods 

//	This function parses a sting query and initializes a Label.
void MAGE::Label::parseQuery( string q )
{
	static string first, middle, last;
	istringstream liness( q );
	
	// parse input label string to get strings : start - end - query
	getline( liness, first,	 ' ' );
	getline( liness, middle, ' ' );
	getline( liness, last,	 ' ' );
	
	// parse splitted strings to get char * : start - end - query
	strcpy( this->labelMemory.strBegin, first.c_str()  );
	strcpy( this->labelMemory.strEnd,	middle.c_str() );
	strcpy( this->labelMemory.strQuery, last.c_str()   );
	
	if( isdigit_string( this->labelMemory.strBegin ) && isdigit_string( this->labelMemory.strEnd ) ) // has correct frame information
	{ 
		this->begin = atof( this->labelMemory.strBegin );
		this->end	= atof( this->labelMemory.strEnd   );
		this->query.assign( this->labelMemory.strQuery );
		this->isDurationForcedFlag = true;
	} 
	else 
	{
		this->begin = -1.0;
		this->end	= -1.0;
		this->query.assign( this->labelMemory.strBegin );
		this->isDurationForcedFlag = false;
	}
	
	this->speed = 1.0;
	

        parsed.parse(this->query.c_str());

	return;
}

//	This function prints the query string of a Label.
void MAGE::Label::printQuery( void )
{	
	printf( "label: %s\n", this->query.c_str() );
	return;
}
