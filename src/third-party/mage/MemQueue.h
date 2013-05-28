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

// TODO :: maybe build that class around pa_ringbuffer instead

#pragma once

#include "pa_memorybarrier.h"
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <unistd.h>

namespace MAGE 
{
	/** 
	 *  \brief		The basic template class for the memory queues used in Mage.
	 *  \details	This class is used to define the memory queue structures that are used in Mage. 
	 *				Memory-efficient lock-free ringbuffer : push and template POD data with memcpy()
	 *				and inform on the state of the buffer
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
	template <class Item>
	class MemQueue 
	{
		public:
	
			/**
			 *	Constructor that allocates the required memory for a MemQueue.
			 *
			 *	@param queueLen Max size of the MemQueue. i.e. how many Item the ringbuffer
			 *				can contain before it's full.
			 */
			MemQueue( unsigned int queueLen );
		
			/**
			 *	Destructor that disallocates the used memory of a MemQueue.
			 */		
			~MemQueue( void );
	
// getters
		
			/**
			 *	This function gets the number of Item currently stored in the queue.
			 * 
			 *	@return The number of Item currently stored (i.e. pushed and not pop'd) in the queue.
			 */
			unsigned int getNumOfItems( void );

// methods
		
			/**
			 *	This function pushes nOfItems Items into the queue by copying them from the Item array *item.
			 * 
			 *	@param item The array of Items to be copied/pushed into the queue.
			 *	@param nOfItems The number of Items to be copied/pushed into the queue.
			 */
			void push( Item * item, unsigned int nOfItems = 1 );
		
			/** 
			 *	This function pushes a new Item into the FIFO queue. In this case, there is no copy of an item,
			 *	instead the next slot of the memory pool is added to the queue. It is supposed that said slot 
			 *  has been accessed with next() and modified beforehand.
			 * 
			 *	Usage is in 3 steps which could be summarized into "access, write, save": 
			 * 
			 *	Item *item = memQueue->next(); //access next available memory slot \n
			 *	item->set...(foo); //modify it \n
			 *	memQueue->push(); //save it into the queue (advance 'write' pointer to next available memory slot) \n
			 * 
			 *	@param nOfItems The number of items to be pushed in the queue, i.e. number of item the
			 *			'write' pointer will be advanced. If no value is provided, 1 (one) item is pushed.
			 *			You will usually want to use it with only one item at a time.
			 * 
			 */
			void push( unsigned int nOfItems = 1 );
		
			/**
			 *	This function copies nItem Items of the FIFO queue into the Item array *item and  
			 *	erase them from the queue. 
			 * 
			 *	@param item A pre-allocated array of Item in which nItem from the queue will be copied.
			 *	@param nOfItems The number of Items to be copied and erased.
			 */
			void pop( Item * item, unsigned int nOfItems = 1 );
		
			/**
			 *	This function removes the oldest item in the queue. In this case, there is no copy of it,
			 *	instead the 'read' pointer that points to the oldest item in the queue is incremented to
			 *  the next item of the queue and the item previously pointed to is returned into the memory pool.
			 * 			 
			 *	Usage is in 3 steps which could be summarized into "access, read, delete":
			 * 
			 *	Item *item = memQueue->get();//access oldest item in the queue \n
			 *	var v = item->get...();//read it, use it, ... \n
			 *	memQueue->pop();//remove it from the queue (advance 'read' pointer) \n
			 *	//never do this after a pop() : \n
			 *	var v2 = item->get...();//don't do this \n
			 * 
			 *	Note that once pop() has been called, the item can be overwritten at any time
			 *	by a subsequent next() / push(). Therefore if the Item has to be used but without
			 *	blocking/clogging up the queue, it is better to make a copy-pop() with pop(Item*, int)
			 * 
			 *	Item *item = memQueue->get();//access oldest item in the queue \n
			 *	memQueue->pop();//remove it from the queue (advance 'read' pointer) \n
			 *	//never do this after a pop() : \n
			 *	var v = item->get...();//don't do this \n
			 * 
			 *	//do this instead \n
			 *	Item item; \n
			 *	memQueue->pop(item);//remove it from the queue (advance 'read' pointer) \n
			 *	var v = item.get...();//item is a local copy of the Item that has been pop()'d \n
			 * 
			 *	@param nOfItems The number of item to be pop()'d from the queue, i.e. number of item the
			 *			'read' pointer will be advanced. If no value is provided, 1 (one) item is pop()'d.
			 *			You will usually want to use it with only one item at a time.
			 */
			void pop( unsigned int nOfItems = 1 );
		
			/**
			 *	This function copies nItem Items of the FIFO queue into the Item array *item. 
			 *	This is meant to be used with pop(unsigned int). 
			 *
			 *	\see pop(unsigned int) doc for more complete explanation.
			 * 
			 *	@param item A pre-allocated array of Item in which nItem from the queue will be copied.
			 *	@param nOfItems The number of Items to be copied.
			 */
			void get( Item * item, unsigned int nOfItems = 1 );
		
			/**
			 *	This function accesses the oldest item of the FIFO queue. This is meant to be used with pop(unsigned int).
			 *
			 *	\see pop(unsigned int) doc for more complete explanation.
			 * 
			 *	@return A pointer to the item of the queue that pop() would remove. i.e. the
			 *				oldest slot in the FIFO.
			 */
			Item * get( void );
		
			/**
			 *	This function accesses the next available slot of the memory pool. This is meant to be 
			 *	used with push(unsigned int). 
			 *
			 *	\see push(unsigned int) doc for more complete explanations.
			 * 
			 *	@return A pointer to the item of the queue that push() would write to. i.e. 
			 *				the next available slot in memory that has not yet been pushed into the FIFO
			 */
			Item * next( void );
	
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
			 *				given to the constructor, false otherwise.
			 */
			bool isFull( void );
	
		protected:

			/**
			 * \var int nOfItems
			 * \brief number of items currently stored in the queue
			 */
			int nOfItems;

			/**
			 * \var unsigned int length
			 * \brief maximum number of items that can be stored in the queue
			 */
			unsigned int length;
			
			/**
			 * \var unsigned int remain
			 * \brief this tells how many elements can be inserted at the "end" of the circular queue before wraping around.
			 * \todo currently this variable can be transformed into a local one wherever it is used. However it is left as a member of the class "just in case" ... for now
			 */
			unsigned int remain;
			
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
			 * \var Item * rawData
			 * \brief the array of Item used as a circular buffer for the queue
			 */
			Item * rawData;
	};
} // namespace

//	Constructor that allocates the required memory for a MemQueue.
template <class Item>
MAGE::MemQueue<Item>::MemQueue( unsigned int queueLen )
{
	length = queueLen;
	rawData = new Item[length];
	
	read = write = 0;
	nOfItems = 0;
}

//	Destructor that disallocates the used memory of a MemQueue.
template <class Item>
MAGE::MemQueue<Item>::~MemQueue( void )
{
	delete[] rawData;
}

//	This function pushes nItem Items into the queue by copying them from the Item array *item.
template <class Item>
void MAGE::MemQueue<Item>::push( Item * item, unsigned int nItem )
{
	// TODO :: fix case when 'write' pointer loops and passes 'read'( read should be advanced to write )
	// this bug shouldn't be triggered as long as we check isfull()before any push()in calling functions
	
	PaUtil_ReadMemoryBarrier();
	
	if( write+nItem <= length )
	{
		//TODO :: either change this for a copy constructor or check that no Item( Model, Frame, ... )has dynamically allocated memory
		memcpy( &rawData[write], item, nItem * sizeof( Item ) );
	} 
	else
	{
		//TODO :: either change this or check that no Item( Model, Frame, ... )has dynamically allocated memory
		remain = length - write;
		memcpy( &rawData[write], item, remain * sizeof( Item ) );
		memcpy( rawData, &item[remain],( nItem-remain ) * sizeof( Item ) );
	}
	
	// TODO :: what if write + nItem > 2 * length ?
	PaUtil_WriteMemoryBarrier();
	write = ( write + nItem ) % length;
	nOfItems += nItem;
	return;
}

//	This function pushes a new Item into the FIFO queue. In this case, there is no copy of an item,
//	instead the next slot of the memory pool is added to the queue. It is supposed that said slot 
//  has been accessed with next() and modified beforehand.
template <class Item>
void MAGE::MemQueue<Item>::push( unsigned int nItem )
{
	// TODO :: fix case when 'write' pointer loops and passes 'read'( read should be advanced to write )
	// this bug shouldn't be triggered as long as we check isfull()before any push()in calling functions
	// TODO :: what if write + nItem > 2 * length ?
	
	PaUtil_WriteMemoryBarrier();
	write = ( write + nItem ) % length;
	nOfItems += nItem;
	return;
}

//	This function copies nItem Items of the FIFO queue into the Item array *item and erase them 
//	from the queue. 
template <class Item>
void MAGE::MemQueue<Item>::pop( Item * item, unsigned int nItem )
{
	// TODO :: fix case when 'write' pointer loops and passes 'read'( read should be advanced to write )
	// this bug shouldn't be triggered as long as we check isfull()before any push()in calling functions
	// TODO :: an isempty()check * is * needed
	// TODO :: check for nItem == 0( shouldn't happen except if isempty() )

	// don't pop()further than available ...
	if( nItem > this->getNumOfItems() )
		nItem = this->getNumOfItems();
	
	PaUtil_ReadMemoryBarrier();
	if( read + nItem <= length )
	{
		memcpy( item, &rawData[read], nItem * sizeof( Item ) );
	} 
	else 
	{
		remain = length - read;
		memcpy( item, &rawData[read], remain * sizeof( Item ) );
		memcpy( &item[remain], rawData,( nItem - remain ) * sizeof( Item ) );
	}
	
	PaUtil_WriteMemoryBarrier();
	read = ( read+nItem ) % length;
	nOfItems -= nItem;
	return;
}

//	This function removes the oldest item in the queue. In this case, there is no copy of it,
//	instead the 'read' pointer that points to the oldest item in the queue is incremented to
//  the next item of the queue and the item previously pointed to is returned into the memory pool.	
template <class Item>
void MAGE::MemQueue<Item>::pop( unsigned int nItem )
{
	if( nItem > this->getNumOfItems() )
		nItem = this->getNumOfItems();
	
	PaUtil_WriteMemoryBarrier();
	read = ( read + nItem ) % length;
	nOfItems -= nItem;
	return;
}

//	This function copies nItem Items of the FIFO queue into the Item array *item. 
template <class Item>
void MAGE::MemQueue<Item>::get( Item * item, unsigned int nItem )
{
	// TODO :: fix case when 'write' pointer loops and passes 'read'( read should be advanced to write )
	// this bug shouldn't be triggered as long as we check isfull()before any push()in calling functions
	// TODO :: an isempty()check * is * needed
	// TODO :: check for nItem == 0( shouldn't happen except if isempty() )

	//don't pop()further than available ...
	if( nItem > this->getNumOfItems() )
		nItem = this->getNumOfItems();
	
	PaUtil_ReadMemoryBarrier();
	if( read + nItem <= length )
	{
		memcpy( item, &rawData[read], nItem * sizeof( Item ) );
	} 
	else
	{
		remain = length - read;
		memcpy( item, &rawData[read], remain * sizeof( Item ) );
		memcpy( &item[remain], rawData,( nItem - remain ) * sizeof( Item ) );
	}
	return;
}

//	This function accesses the oldest item of the FIFO queue. This is meant to be used with pop(int).
template <class Item>
Item * MAGE::MemQueue<Item>::get( void )
{
	return &rawData[read];
}

//	This function accesses the next available slot of the memory pool. This is meant to be 	used with push(int).
template <class Item>
Item * MAGE::MemQueue<Item>::next( void )
{
	return &rawData[write];
}

//	This function returns true if the queue is empty, false otherwise. 
template <class Item>
bool MAGE::MemQueue<Item>::isEmpty( void )
{
	PaUtil_ReadMemoryBarrier();
	return( nOfItems <= 0 ) ? true : false;
}

//	This function returns true if the queue is full, i.e. if it contains the max number 
//	of elements given to the constructor, false otherwise.
template <class Item>
bool MAGE::MemQueue<Item>::isFull( void )
{
	PaUtil_ReadMemoryBarrier();
	return( this->getNumOfItems() >= length ) ? true : false;
}

//	This function gets the number of Item currently stored in the queue.
template <class Item>
unsigned int MAGE::MemQueue<Item>::getNumOfItems( void )
{
	return( unsigned int )( nOfItems > 0 ) ? nOfItems : 0;
}

