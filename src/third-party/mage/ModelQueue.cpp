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

#include "ModelQueue.h"

#ifdef WIN32
extern "C" __declspec(dllimport) void __stdcall Sleep(unsigned long);
#endif

//	Constructor that allocates the required memory for a Model queue.
MAGE::ModelQueueMemory::ModelQueueMemory()
{
	int k;

	// for every stream, for every frame, every mean / ivar / optimized parameters / gv_mean / gv_vari / gv_switch
	this->mean	= ( double *** ) calloc( nOfStreams, sizeof( double ** ) );	// [nOfStreams][maxNumOfFrames][maxStreamLen]
	this->ivar	= ( double *** ) calloc( nOfStreams, sizeof( double ** ) );	// [nOfStreams][maxNumOfFrames][maxStreamLen]
	this->par	= ( double *** ) calloc( nOfStreams, sizeof( double ** ) );	// [nOfStreams][maxNumOfFrames][maxStreamLen]
	
	this->gv_mean	= ( double ** ) calloc( nOfStreams, sizeof( double * ) );	// [nOfStreams][maxStreamLen]
	this->gv_vari	= ( double ** ) calloc( nOfStreams, sizeof( double * ) );	// [nOfStreams][maxStreamLen]
	this->gv_switch = ( int    ** ) calloc( nOfStreams, sizeof( int   * ) );	// [nOfStreams][maxStreamLen]
	
	this->voiced_unvoiced = ( int * ) calloc( maxNumOfFrames, sizeof( int ) );	// [maxNumOfFrames]
	
	// HTS106_SMatrixies needed
	this->g		= ( double **  ) calloc( nOfStreams, sizeof( double * ) );	// [nOfStreams][maxNumOfFrames]
	this->wum	= ( double **  ) calloc( nOfStreams, sizeof( double * ) );	// [nOfStreams][maxNumOfFrames]
	this->wuw	= ( double *** ) calloc( nOfStreams, sizeof( double ** ) );	// [nOfStreams][maxNumOfFrames][maxWindowWidth]
	
	for( k = 0; k < nOfStreams; k++ )
	{
		this->mean[k] = ( double ** ) calloc( maxNumOfFrames, sizeof( double * ) );
		this->ivar[k] = ( double ** ) calloc( maxNumOfFrames, sizeof( double * ) );
		this->par [k] = ( double ** ) calloc( maxNumOfFrames, sizeof( double * ) );
		this->wuw [k] = ( double ** ) calloc( maxNumOfFrames, sizeof( double * ) );
		this->wum [k] = ( double * ) calloc( maxNumOfFrames, sizeof( double   ) );
		this->g	  [k] = ( double * ) calloc( maxNumOfFrames, sizeof( double   ) );
		
		this->gv_mean[k]	= ( double * ) calloc( nOfStreams * maxStreamLen, sizeof( double ) );	
		this->gv_vari[k]	= ( double * ) calloc( nOfStreams * maxStreamLen, sizeof( double ) );	
		this->gv_switch[k]	= ( int    * ) calloc( nOfStreams * maxStreamLen, sizeof( int    ) );	
		
		for( int j = 0; j < maxNumOfFrames; j++ )
		{
			this->mean[k][j] = ( double * ) calloc( maxStreamLen, sizeof( double ) );
			this->ivar[k][j] = ( double * ) calloc( maxStreamLen, sizeof( double ) );
			this->par [k][j] = ( double * ) calloc( maxStreamLen, sizeof( double ) );
			this->wuw [k][j] = ( double * ) calloc( maxWindowWidth   , sizeof( double ) );
		}
	}	
}

//	Destructor that disallocates all the memory used from a Model queue.
MAGE::ModelQueueMemory::~ModelQueueMemory( void )
{
	int k;
	
	for( k = 0; k < nOfStreams; k++ )
	{
		for( int j = 0; j < maxNumOfFrames; j++ )
		{
			free( this->mean[k][j] );
			free( this->ivar[k][j] );
			free( this->par [k][j] );
			free( this->wuw [k][j] );
		}
		
		free( this->mean[k] );
		free( this->ivar[k] );
		free( this->par [k] );
		free( this->g	[k] );
		free( this->wum [k] );
		free( this->wuw [k] );
		free( this->gv_mean	 [k] );
		free( this->gv_vari	 [k] );
		free( this->gv_switch[k] );
	}
	
	free( this->mean );
	free( this->ivar );
	free( this->par  );
	free( this->g	 );
	free( this->wum  );
	free( this->wuw  );
	free( this->gv_mean );
	free( this->gv_vari );
	free( this->gv_switch );
	free( this->voiced_unvoiced );
}


//	Constructor that allocates the required memory for a ModelQueue.
MAGE::ModelQueue::ModelQueue( unsigned int queueLen ):
MAGE::MemQueue<Model>( queueLen )
{
	//this->modelQueueMemory = new MAGE::ModelQueueMemory::ModelQueueMemory();
	// this is just to pass the
	// queueLen to the parent class
}

//	Destructor that disallocates all the memory used from a ModelQueue.
MAGE::ModelQueue::~ModelQueue()
{
}

// methods
//	This function generates the PDF parameters of every stream.
void MAGE::ModelQueue::generate( MAGE::Engine * engine, FrameQueue * frameQueue, unsigned int backup )
{
	HTS106_ModelSet ms = engine->getModelSet();
	HTS106_Global global = engine->getGlobal();

	//TODO :: actual frame generation with vocoder
	unsigned int i, k, s, q, w, ind;
	unsigned int qStream[nOfStreams];
	
	head = ( read + backup ) % length;	// then we land on the correct model( backup models 
										// have already been used, let's forget about them )
	for( i = 0; i < nOfStreams; i++ )
		qStream[i] = 0;
	
	for( w = 0; w < backup; w++ )
	{
		ind = ( read + w ) % length;
		
		for( s = 0; s < nOfStates; s++ )
		{
			//this is totally idiotic because it has already been computed in optimizeparameters
			//thus it should be saved there and re-used here
			for( i = 0; i < nOfStreams; i++ )
			{
				if( i != lf0StreamIndex )
					qStream[i] += rawData[ind].getState( s ).duration;
				else
					if( rawData[ind].getState( s ).streams[i][0].msdFlag > global.msd_threshold[i] )
						qStream[i] += rawData[ind].getState( s ).duration;
			}
		}
	}
	
	for( s = 0; s < nOfStates; s++ )
	{
		// from each state of the model, we get the computed
		// duration and we iterate to generate the parameters

		for( q = 0; q < rawData[head].getState( s ).duration; q++ )
		{
			while( frameQueue->isFull() )
                        #ifdef WIN32
                          Sleep(1);
                        #else
                          usleep( 1000 );
                        #endif
			
			frame = frameQueue->next();
			
			for( i = 0; i < nOfStreams; i++ )
			{
				int index = qStream[i];
	
				if ( i == lf0StreamIndex )
				{
					if( rawData[head].getState( s ).streams[i][0].msdFlag > global.msd_threshold[i] )
					{						
						frame->voiced = true;
						
                                                #ifdef RHVOICE
                                                frame->streams[i][0]=modelQueueMemory.par[i][index][0];
                                                #else
						if( useLF0 )
							frame->streams[i][0] = exp( this->modelQueueMemory.par[i][index][0] );
						
						if( useMELF0 )
							frame->streams[i][0] = 700 * (exp ( this->modelQueueMemory.par[i][index][0] / 1127 ) - 1 );
                                                #endif
						
						qStream[i]++;
					} 
					else
					{
						frame->voiced = false;
						frame->streams[i][0] = 0;
					}
				}
				else
				{	
					//TODO :: memcpy ? (faster ?)
					for( k = 0; k < ms.stream[i].vector_length; k++ )
						frame->streams[i][k] = this->modelQueueMemory.par[i][index][k];
					qStream[i]++;

					if( enableMLSAcheck && i == mgcStreamIndex)
					{
						mlsacheck(frame->streams[mgcStreamIndex], nOfMGCs-1, fftLen, index, defaultAlpha, R1, R2, true, 0);
					}
				}		
			}
			
			frameQueue->push();
		}
	}
	return;
}

//	This function optimizes the generated parameters of every stream.
void MAGE::ModelQueue::optimizeParameters( MAGE::Engine * engine, unsigned int backup, unsigned int lookup )
{	
	int window = backup + lookup + 1;//how many model do we use
	head = read; // hopefuly we land on the oldest model wich is 'backup' earlier than current model
	
	HTS106_ModelSet ms = engine->getModelSet();
	HTS106_Global global = engine->getGlobal();
	HTS106_PStream pss = engine->getPStream();
	
	int i, j, k, l, m, w;
	int state, frame, msd_frame, static_length;
	int total_duration = 0;
	
	bool not_bound;
	
	frame = 0;
	
	//TODO :: optimize this: some computation will be done exactly the same in next 
	//call to this function( only shifted in mem )
	
	for( w = 0; w < window; w++ )	// for every model 
	{ 
		for( state = 0; state < nOfStates; state++ )// for every state
		{
			for( j = 0; j < rawData[head].getState( state ).duration; j++, frame++ )// for every frame
			{
				if( rawData[head].getState( state ).streams[lf0StreamIndex][0].msdFlag > global.msd_threshold[lf0StreamIndex] )	// if voiced
					this->modelQueueMemory.voiced_unvoiced[frame] = true;
				else
					this->modelQueueMemory.voiced_unvoiced[frame] = false;
			}
		}
		
		total_duration += rawData[head].getDuration();
		
		head = ( head + 1 ) % length;
	}
	
	for( i = 0; i < nOfStreams; i++ ) // for every stream : mgc, lf0, bap
	{
		head = read;
		
		if( ms.stream[i].msd_flag )	// for MSD
		{
			frame = 0, msd_frame = 0;
			for( w = 0; w < window; w++ )
			{
				for( state = 0; state < nOfStates; state++ ) // for every state
				{
					for( j = 0; j < rawData[head].getState( state ).duration; j++ ) // for every frame
					{
						if( this->modelQueueMemory.voiced_unvoiced[frame] )
						{
							// check current frame is MSD boundary or not 
							for( k = 0; k < ms.stream[i].window.size; k++ )
							{
								not_bound = true;
								
								for( l = ms.stream[i].window.l_width[k]; l <= ms.stream[i].window.r_width[k]; l++ )
								{
									if( frame + l < 0 || total_duration <= frame + l || !this->modelQueueMemory.voiced_unvoiced[frame+l] ) // voiced unvoided
									{
										not_bound = false;
										break;
									}
								}
								
								static_length = ( ms.stream[i].vector_length / ms.stream[i].window.size );
								
								for( l = 0; l < static_length; l++ )
								{
									m = static_length * k + l;
									
									this->modelQueueMemory.mean[i][msd_frame][m] = rawData[head].getState( state ).streams[i][m].mean;
										
									if( not_bound || k == 0 )
										this->modelQueueMemory.ivar[i][msd_frame][m] = HTS106_finv( rawData[head].getState( state ).streams[i][m].vari );
									else
										this->modelQueueMemory.ivar[i][msd_frame][m] = 0.0;
										
									// ATTENTION :: Loop assignment
									this->modelQueueMemory.gv_mean[i][m] = rawData[head].getState( 0 ).gv_streams[i][m].mean;
									this->modelQueueMemory.gv_vari[i][m] = rawData[head].getState( 0 ).gv_streams[i][m].vari;
								}
							}
							this->modelQueueMemory.gv_switch[i][msd_frame] = rawData[head].getState( state ).gv_switch_streams[i];
							msd_frame++;
						}
						frame++;
					}
				}
				head = ( head + 1 ) % length;
			}
		} 
		else // for non MSD 
		{			
			frame = 0;
			
			for( w = 0; w < window; w++ )
			{
				for( state = 0; state < nOfStates; state++ ) // for every state
				{
					for( j = 0; j < rawData[head].getState( state ).duration; j++ ) // for every frame
					{
						for( k = 0; k < ms.stream[i].window.size; k++ )
						{
							not_bound = true;
							
							for( l = ms.stream[i].window.l_width[k]; l <= ms.stream[i].window.r_width[k]; l++ )
							{
								if( frame + l < 0 || total_duration <= frame + l )
								{
									not_bound = false;
									break;
								}
							}
							
							static_length = ( ms.stream[i].vector_length / ms.stream[i].window.size );
							
							for( l = 0; l < static_length; l++ )
							{
								m = static_length * k + l;
								
								this->modelQueueMemory.mean[i][frame][m] = rawData[head].getState( state ).streams[i][m].mean;

								if( not_bound || k == 0 )
									this->modelQueueMemory.ivar[i][frame][m] = HTS106_finv( rawData[head].getState( state ).streams[i][m].vari );
								else
									this->modelQueueMemory.ivar[i][frame][m] = 0.0;
									
								this->modelQueueMemory.gv_mean[i][m] = rawData[head].getState( 0 ).gv_streams[i][m].mean;
								this->modelQueueMemory.gv_vari[i][m] = rawData[head].getState( 0 ).gv_streams[i][m].vari;
								this->modelQueueMemory.gv_switch[i][m] = rawData[head].getState( 0 ).gv_switch_streams[i];
							}
						}
						frame++;
					}
				}
				head = ( head+1 )%length;
			}
		}
		
		// just vector assigments in order to create a HTS106_PStream object and use the default HTS106_PStream_mlpg function
		pss.vector_length = ms.stream[i].vector_length;			// vector length( include static and dynamic features )
		pss.width = ms.stream[i].window.max_width * 2 + 1;		// width of dynamic window 
		pss.win_size = ms.stream[i].window.size;				// # of windows( static + deltas )
		pss.win_l_width = ms.stream[i].window.l_width;			// left width of windows 
		pss.win_r_width = ms.stream[i].window.r_width;			// right width of windows 
		pss.win_coefficient = ms.stream[i].window.coefficient;	// window coefficients 
		pss.gv_mean = this->modelQueueMemory.gv_mean[i];							// mean vector of GV 
		pss.gv_vari = this->modelQueueMemory.gv_vari[i];							// variance vector of GV 
		pss.gv_switch = this->modelQueueMemory.gv_switch[i];						// GV flag sequence 
		pss.static_length = ms.stream[i].vector_length / ms.stream[i].window.size;		// static features length 
		
		
		if( ms.stream[i].msd_flag )	// for MSD :: lf0
			pss.length = msd_frame;	// stream length :: total number of frames
		else						// for non MSD :: mgcs & bap
			pss.length = frame;		// stream length :: total number of frames
		
		if( HTS106_ModelSet_use_gv( &ms, i ) )	// if GV is used 
			pss.gv_length = pss.length;		//frame length for GV calculation
		else 
			pss.gv_length = 0;				//frame length for GV calculation
		
		// matrices for parameter generation 		
		pss.sm.mean = this->modelQueueMemory.mean[i];	// mean vector sequence 
		pss.sm.ivar = this->modelQueueMemory.ivar[i];	// inverse diag variance sequence
		pss.sm.g	= this->modelQueueMemory.g[i];		// vector used in the forward substitution 
		pss.sm.wum	= this->modelQueueMemory.wum[i];		// W' U^-1 mu 
		pss.sm.wuw	= this->modelQueueMemory.wuw[i];		// W' U^-1 W	
		
		// output parameter vector in a pre-allocated memory
		pss.par = this->modelQueueMemory.par[i];		 
		
		HTS106_PStream_mlpg( &pss );			// parameter generation 
	}

	return;
}

//	This function prints the content of a ModelQueue.
void MAGE::ModelQueue::printQueue( void )
{
	unsigned int k, s;
	
	for( k = 0; k < getNumOfItems(); k++ )
	{
		int head = ( read + k ) % length;
		
		printf( "model %i:", head );
		
		for( s = 0; s < nOfStates; s++ )
			printf( " %i", rawData[head].getState( s ).duration );
		
		printf( "\n" );
	}
	return;
}

