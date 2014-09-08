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

#include "LabelQueue.h"

//	Constructor that allocates the required memory for a LabelQueue.
MAGE::LabelQueue::LabelQueue( unsigned int size )
{
	queue.resize( size );
	read = write = 0;
	nOfLabels = 0;
}

// methods
//	This function pushes a new Label instance into the FIFO queue by copying it.
void MAGE::LabelQueue::push( Label &label )
{
	queue[write] = label;
	
	write = ( write + 1 ) % queue.size();
	PaUtil_WriteMemoryBarrier();
	nOfLabels++;
	return;
}

//	This function pushes a new MAGE::Label into the FIFO queue. In this case, there is 
//	no copy of a label, instead the next slot of the memory pool is added to the queue. 
//	It is supposed that said slot has been accessed with next() and modified beforehand.
void MAGE::LabelQueue::push( void )
{
	write = ( write + 1 ) % queue.size();
	PaUtil_WriteMemoryBarrier();
	nOfLabels++;
	return;
}

//	This function removes the oldest item in the queue.
void MAGE::LabelQueue::pop( Label &label )
{
	label = queue[read];
	
	read = ( read + 1 ) % queue.size();
	PaUtil_WriteMemoryBarrier();
	nOfLabels--;
	return;
}

//	This function removes the oldest item in the queue. In this case, there is no copy 
//	of a label, instead the 'read' pointer that points to the oldest item in the queue
//	is incremented to the next item of the queue and the item previously pointed to is
//	returned into the memory pool.
void MAGE::LabelQueue::pop( void )
{	
	read = ( read + 1 ) % queue.size();
	PaUtil_WriteMemoryBarrier();
	nOfLabels--;
	return;
}

//	This function is like pop(Label&) but does not advance in the queue.
void MAGE::LabelQueue::get( Label &label )
{
	label = queue[read];
	return;
}

//	This function accesses the oldest item of the FIFO queue. This is meant to be used with pop(void).
MAGE::Label * MAGE::LabelQueue::get( void )
{
		return &queue[read];
}

MAGE::Label * MAGE::LabelQueue::next( void )
{
		return &queue[write];
}

// accessors 
//	This function returns true if the queue is empty, false otherwise. 
bool MAGE::LabelQueue::isEmpty( void )
{
	PaUtil_ReadMemoryBarrier();
	
	if( nOfLabels <= 0 )
		return true;
	else 
		return false;
}

//	This function returns true if the queue is full, i.e. if it contains the max number 
//	of elements given to the constructor, false otherwise. 
bool MAGE::LabelQueue::isFull( void )
{
	PaUtil_ReadMemoryBarrier();
	
	if( nOfLabels >= queue.size() )
		return true;
	else 
		return false;
}

