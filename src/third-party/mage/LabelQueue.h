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

#include <vector>
#include "Label.h"
#include "pa_memorybarrier.h"

using namespace std;

namespace MAGE 
{
	/** 
	 *  \brief		The memory queue of Label instances used in Mage (Label ringbuffer).
	 *  \details	This class is used to exchange the Label instances between the different threads; 
	 *				we could not inherit from MemQueue because Label is not a POD type.
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
	class LabelQueue 
	{
		public:
	
			/**
			 *	Constructor that allocates the required memory for a LabelQueue.
			 * 
			 *	@param size The max size of the LabelQueue. i.e. how many Label the ringbuffer 
			 *					can contain before it's full.
			 */
			LabelQueue( unsigned int size );
	
// methods
		
			/**
			 *	This function pushes a new Label instance into the FIFO queue by copying it.
			 * 
			 *	@param label An instance of a Label that will be copied into the youngest
			 *					item of the memory queue.
			 */
			void push( Label &label );
		
			/**
			 *	This function pushes a new MAGE::Label into the FIFO queue. In this case, there is 
			 *	no copy of a label, instead the next slot of the memory pool is added to the queue. 
			 *	It is supposed that said slot has been accessed with next() and modified beforehand.
			 *
			 *	Usage is in 3 steps which could be summarized into "access, write, save":
			 * 
			 *	Label *label = labelQueue->next();//access next memory slot \n
			 *	label->setQuery("ae^l-ax+s=w@...");//modify it \n
			 *	labelQueue->push();//save it into the queue (advance 'write' pointer) \n
			 * 
			 */
			void push( void );
		
			/**
			 *	This function removes the oldest item in the queue and copies it into a Label variable.
			 * 
			 *	@param label An instance of Label into which the pop()'d label will be 
			 *					copied before being removed from the queue
			 */
			void pop ( Label &label );
		
			/**
			 *	This function removes the oldest item in the queue. In this case, there is no copy 
			 *	of a label, instead the 'read' pointer that points to the oldest item in the queue
			 *	is incremented to the next item of the queue and the item previously pointed to is
			 *  returned into the memory pool.
			 *
			 *	Usage is in 3 steps which could be summarized into "access, read, delete":
			 * 
			 *	Label *label = labelQueue->get();//access oldest item in the queue \n
			 *	string s = label->getQuery();//read it, use it, ... \n
			 *	labelQueue->pop();//remove it from the queue (advance 'read' pointer) \n
			 *	//never do this after a pop() : \n
			 *	string s2 = label->getQuery();//don't do this \n
			 * 
			 *	Note that once pop() has been called, the item can be overwritten at any time
			 *	by a subsequent next() / push(). Therefore if the Label has to be used but without
			 *	blocking/clogging up the queue, it is better to make a copy-pop() with pop(Label&)
			 * 
			 *	Label *label = labelQueue->get();//access oldest item in the queue \n
			 *	labelQueue->pop();//remove it from the queue (advance 'read' pointer) \n
			 *	//never do this after a pop() : \n
			 *	string s = label->getQuery();//don't do this \n
			 * 
			 *	//do this instead \n
			 *	Label label; \n
			 *	labelQueue->pop(label);//remove it from the queue (advance 'read' pointer) \n
			 *	string s = label.getQuery();//label is a local copy of the label that has been pop()'d \n
			 */
			void pop( void );
		
			/**
			 *	This function is like pop(Label&) but does not advance in the queue.
			 *
			 *	@param label An instance of Label into which the get()'d label will be copied.
			 */
			void get ( Label &label );
		
			/**
			 *	This function accesses the oldest item of the FIFO queue. This is meant to be used with pop(void).
			 *	\see pop(void) doc for more complete explanation.
			 * 
			 *	@return A pointer to the item of the queue that pop() would remove. i.e. the
			 *			oldest slot in the FIFO
			 */
			Label * get( void );
		
			/**
			 *	This function accesses the next available slot of the memory pool. This is meant to 
			 *	be used with push(void). see push(void) doc for more complete explanations.
			 * 
			 *	@return A pointer to the item of the queue that push() would write to. i.e. 
			 *			the next available slot in memory that has not yet been pushed into the FIFO.
			 */
			Label * next( void );

// accessors 

			/**
			 *	This function returns true if the queue is empty, false otherwise. 
			 *
			 *	@return True if the queue is empty, false otherwise.
			 */
			bool isEmpty( void );
		
			/**
			 *	This function returns true if the queue is full, i.e. if it contains the max number 
			 *	of elements given to the constructor, false otherwise. 
			 * 
			 *	@return True if the queue is full, i.e. if it contains the max number of elements
			 *			given to the constructor, false otherwise.
			 */
			bool isFull ( void );
	
	
		protected:
	
			/**
			 * \var std::vector<Label> queue
			 * \brief it contains the labels pushed into the queue (for instance labels extracted from an HTS label file)
			 */
			vector<Label> queue;

			/**
			 * \var unsigned int read
			 * \brief index of the position where the next get() / pop() will happen
			 */
			unsigned int read;

			/**
			 * \var unsigned int write
			 * \brief index of the position where the next next() / push() will happen
			 */
			unsigned int write;

			/**
			 * \var unsigned int nOfLabels
			 * \brief number of Label currently stored in the queue
			 */
			unsigned int nOfLabels;
	};
} // namespace
