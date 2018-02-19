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

#include <string>
#include <cstring>
#include <sstream>

#include "hts.h"
#include "Constants.h"

#include "core/question_matcher.h"

using namespace std;

namespace MAGE 
{
	/** 
	 *  \brief		The memory used for every Label query.
	 *  \details	This class is used to define the memory structures that are needed for a Label query.
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
	class LabelMemory 
	{
		public :
		
			/**
			 *	Constructor that allocates the required memory for a single Label query.
			 */
			LabelMemory();
		
			/**
			 *	Destructor that disallocates all the memory used from a single Label query.
			 */	
			~LabelMemory();
		
			/** 
			 *	\var char strQuery.
			 *	\brief It contains the actual string query.
			 */
			char strQuery[maxStrLen];
		
			/** 
			 *	\var char strBegin.
			 *	\brief It contains the starting time of the given query.
			 */
			char strBegin[maxStrLen];
		
			/** 
			 *	\var char strEnd.
			 *	\brief It contains the ending time of the given query.
			 */
			char strEnd[maxStrLen];
	};
	
	/** 
	 *  \brief     The Label query used to retrieve the statistical information for building a Model.
	 *  \details   This class is used to define the Label query structures, such as the label string,
	 *				the time tags and whether the duration is forced by the user.
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
	class Label 
	{
		public:
	
			/**
			 *	Constructor that initializes all the parameters needed for a single Label query in default values.
			 */	
			Label( void );
		
			/**
			 *	Constructor that initializes all the parameters needed for a single Label query given a string query.
			 *
			 *	@param query An string query.
			 */	
			Label( string query );
		
			/**
			 *	Destructor that disallocates all the memory used by a Label.
			 */		
			~Label( void );
	
// getters
		
			/**
			 *	This function gets the current query used in the Label.
			 *
			 *	@return The query value used in the Label.
			 */
			inline string getQuery( void ){ return( this->query ); };
		
			/**
			 *	This function gets the current speed used in the Label.
			 *
			 *	@return The speed value used in the Label.
			 */
			inline double getSpeed( void ){ return( this->speed ); };

			/**
			 *	This function gets the current starting time of the query used in the Label.
			 *
			 *	@return The starting time of the query used in the Label.
			 */
			inline int getBegin( void ){ return( this->begin ); };
		
			/**
			 *	This function gets the current ending time of the query used in the Label.
			 *
			 *	@return The ending time of the query used in the Label.
			 */
			inline int getEnd  ( void ){ return( this->end   ); };


//setters

                        inline const RHVoice::parsed_label_string& getParsed() const {return parsed;}


			/**
			 *	This function sets a new value of the query to be used in the Label.
			 *
			 *	@param query The new value of the query.
			 */		
			void setQuery( string query );
		
			/**
			 *	This function sets a new value of the starting time of the query to be used in the Label.
			 *
			 *	@param begin The new starting time value of the query.
			 */	
			inline void setBegin( int begin ){ this->begin = begin; };
		
			/**
			 *	This function sets a new value of the ending time of the query to be used in the Label.
			 *
			 *	@param end The new value ending time of the query.
			 */	
			inline void setEnd( int end ){ this->end = end; };
		
			/**
			 *	This function sets a new value of the speed in the Label.
			 *
			 *	@param speed The new speed value of the query.
			 */	
			inline void setSpeed( double speed ){ this->speed = speed; };
		
			/**
			 *	This function sets the durations to be forced by the user or generated by the Model.
			 *
			 *	@param isDurationForcedFlag True if the duration of the Label is forced by the user and false otherwise.
			 */
			inline void setDurationForced( bool isDurationForcedFlag ){ this->isDurationForcedFlag = isDurationForcedFlag; };
		
// methods
		
			/** 
			 *	This function prints the query string of a Label.
			 */
			void printQuery( void );
		
			/** 
			 *	This function parses a sting query and initializes a Label.
			 * 
			 *	@param query A string query.
			 */
			void parseQuery( string query );

// accessors
		
			/**
			 *	This function returns true if a duration of the Label is forced by the user
			 *		and false if it is generated by the Model.
			 *
			 *	@return True if the duration of the Label is forced by the user and false otherwise.
			 */		
			inline bool isDurationForced( void ){ return( this->isDurationForcedFlag ); };
		
		protected:
	
			/** 
			 *	\var LabelMemory labelMemory.
			 *	\brief It contains the memory used by a Label.
			 */
			LabelMemory labelMemory;

			/** 
			 *	\var string query.
			 *	\brief It contains the string query used by a Label.
			 */
			string query;
		
			/** 
			 *	\var bool isDurationForcedFlag.
			 *	\brief It contains the information of the way the duration of the Model 
			 *			will be generated; either forced by the user or genereted by the Model.
			 */
			bool isDurationForcedFlag;
		
			/** 
			 *	\var double speed.
			 *	\brief It contains the speed used by a Label.
			 */
			double speed;
		
			/** 
			 *	\var int begin.
			 *	\brief It contains the starting time of the query used by a Label.
			 */
			int begin;
		
			/** 
			 *	\var int end.
			 *	\brief It contains the ending time of the query used by a Label.
			 */
			int end;

                        RHVoice::parsed_label_string parsed;
	};
} // namespace
