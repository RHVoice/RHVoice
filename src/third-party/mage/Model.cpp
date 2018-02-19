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

#include "Model.h"

// constructor
MAGE::ModelMemory::ModelMemory()
{
	int k;
	
	// for every state
	this->duration_mean  = ( double * ) calloc( nOfStates, sizeof( double ) );
	this->duration_vari  = ( double * ) calloc( nOfStates, sizeof( double ) );
	this->duration_array = ( int    * ) calloc( nOfStates, sizeof( int    ) );;
	
	// # of MGC coefficients for the MLSA filter & # of derivations: ∆( 0 ), ∆( 1 ), ∆( 3 )
	// fundamental frequency & # of derivations: ∆( 0 ), ∆( 1 ), ∆( 3 )
	// # of low-pass filter coefficients & # of derivations: ∆( 0 ), ∆( 1 ), ∆( 3 )
	this->stream_mean	= ( double ** ) calloc( nOfStreams, sizeof( double * ) );	// [nOfStreams]
	this->stream_vari	= ( double ** ) calloc( nOfStreams, sizeof( double * ) );	// [nOfStreams]
	
	for( k = 0; k < nOfStreams; k++ )
	{
		this->stream_mean[k] = ( double * ) calloc( maxStreamLen, sizeof( double ) );	
		this->stream_vari[k] = ( double * ) calloc( maxStreamLen, sizeof( double ) );	
	}
}

// destructor
MAGE::ModelMemory::~ModelMemory( void )
{
	for( int k = 0; k < nOfStreams; k++ )
	{
		free( this->stream_mean[k] );
		free( this->stream_vari[k] );
	}

	free( this->duration_mean  ); 
	free( this->duration_vari  ); 
	free( this->duration_array ); 
	free( this->stream_mean ); 
	free( this->stream_vari ); 
}

// constructor
MAGE::Model::Model()
{
	this->duration = 0;
	this->weightsChecked = false;
}

// destructor
MAGE::Model::~Model( void )
{
}

// getters
const MAGE::State& MAGE::Model::getState( int index ) const
{
	return( this->state[index] );
}

int MAGE::Model::getDuration( void )
{ 
	return( this->duration );
}

//setters
void MAGE::Model::setState( MAGE::State state, int index )
{
	this->state[index] = state;
	return;
}

void MAGE::Model::setDuration( int duration )
{
	this->duration = duration;
	return;
}

// methods
void MAGE::Model::checkInterpolationWeights( MAGE::Engine * engine, bool forced )// ATTENTION :: it must be run at least once
{
	int i, j;
	double temp;
	
	HTS106_ModelSet ms = engine->getModelSet();
	HTS106_Global global = engine->getGlobal();
	
	//do it only once
	if( !this->weightsChecked || forced )
	{

		// check interpolation weights 
		for( i = 0, temp = 0.0; i < HTS106_ModelSet_get_duration_interpolation_size( &ms ); i++ )
			temp += global.duration_iw[i];

		if( temp != 0.0 )
			for( i = 0; i < HTS106_ModelSet_get_duration_interpolation_size( &ms ); i++ )
				if( global.duration_iw[i] != 0.0 )
					global.duration_iw[i] /= temp; // ATTENTION :: should not change in the model !!!

		for( i = 0; i < nOfStreams; i++ )
		{
			for( j = 0, temp = 0.0; j < HTS106_ModelSet_get_parameter_interpolation_size( &ms, i ); j++ )
				temp += global.parameter_iw[i][j];

			if( temp != 0.0 )
				for( j = 0; j < HTS106_ModelSet_get_parameter_interpolation_size( &ms, i ); j++ )
					if( global.parameter_iw[i][j] != 0.0 )
						global.parameter_iw[i][j] /= temp; // ATTENTION :: should not change in the model !!!

			if( HTS106_ModelSet_use_gv( &ms, i ) )
			{
				for( j = 0, temp = 0.0; j < HTS106_ModelSet_get_gv_interpolation_size( &ms, i ); j++ )
					temp += global.gv_iw[i][j];

				if( temp != 0.0 )
					for( j = 0; j < HTS106_ModelSet_get_gv_interpolation_size( &ms, i ); j++ )
						if( global.gv_iw[i][j] != 0.0 )
							global.gv_iw[i][j] /= temp; // ATTENTION :: should not change in the model !!!
			}
		}
		this->weightsChecked = true;
	}
	return;
}

void MAGE::Model::computeDuration( MAGE::Engine * engine, MAGE::Label * label, double * interpolationWeight )
{
	int	i;
	double iw;
	double temp;
	double frame_length;

	HTS106_ModelSet ms = engine->getModelSet();
	HTS106_Global global = engine->getGlobal();
	
	// determine state duration 
	const double rate = global.sampling_rate / ( global.fperiod * 1e+7 );
	
	// convert string query to char * 
	string query = label->getQuery();
	strcpy( this->modelMemory.strQuery, query.c_str() );
	
	if( interpolationWeight != NULL )
		iw = interpolationWeight[durStreamIndex];
	else 
		iw = 1;
	
	// HTS106_ModelSet_get_duration: get duration using interpolation weight 
	HTS106_ModelSet_get_duration( &ms, this->modelMemory.strQuery, label->getParsed().get_data(), this->modelMemory.duration_mean, 
									this->modelMemory.duration_vari, global.duration_iw );
	
	if( label->isDurationForced() ) // use duration set by user : -vp
	{
		frame_length = ( label->getEnd()- label->getBegin() ) * rate;
		
		if( label->getEnd() > 0 )
			this->duration = mHTS106_set_duration( this->modelMemory.duration_array, this->modelMemory.duration_mean,
												this->modelMemory.duration_vari , nOfStates, frame_length );
		else
			HTS106_error( -1,( char * )"HTS106_SStreamSet_create: The time of final label is not specified.\n" );
	}
	else // determine frame length
	{
		if( label->getSpeed()!= 1 )
		{
			temp = 0.0;
			for( i = 0; i < nOfStates; i++ )
				temp += this->modelMemory.duration_mean[i];
			
			frame_length = temp / label->getSpeed();
                        if(frame_length<nOfStates)
                          frame_length=nOfStates;
		}
		else
			frame_length = 0;
		
		// set state duration 
		this->duration = mHTS106_set_duration( this->modelMemory.duration_array, this->modelMemory.duration_mean, 
											this->modelMemory.duration_vari, nOfStates, frame_length );
	}
	
	for( i = 0; i < nOfStates; i++ )
	{
		this->state[i].duration += Round(iw * this->modelMemory.duration_array[i]);
		
		//printf("state[%d] Dur = %d \n", i, this->state[i].duration);
	}
	return;
}

void MAGE::Model::updateDuration( double * updateFunction, int action )
{
	int duration = 0;
	
	if( updateFunction == NULL )
		return;
	
	for( int i = 0; i < nOfStates; i++ )
	{
		switch( action )
		{
			case MAGE::overwrite:
				this->state[i].duration = updateFunction[i];
				break;
				
			case MAGE::shift:
				this->state[i].duration += updateFunction[i];
				break;
				
			case MAGE::scale:
				this->state[i].duration *= updateFunction[i];
				break;
				
			case MAGE::synthetic:
			case MAGE::noaction:
			default:
				break;
		}
		
		if( this->state[i].duration < 0)
			this->state[i].duration = 1;
		
		duration += this->state[i].duration;
	}
	
	//fix for memory overflow (ModelQueueMemory has only maxNumOfFrames allocated)
	//see maxDuration in Constant.h
	if( duration > maxDuration )
	{
		int tmp = 0;
		for( int i = 0; i < nOfStates; i++ )
		{
			this->state[i].duration *= maxDuration;
			this->state[i].duration /= duration;//(int) --> will round to floor
			tmp += this->state[i].duration;
		}
		duration = tmp;
	}
	
	this->duration = duration;
	return;
}

void MAGE::Model::computeParameters( MAGE::Engine * engine, MAGE::Label * label, double * interpolationWeight ) // for every stream
{
	int i, j, k;
	
	HTS106_ModelSet ms = engine->getModelSet();
	HTS106_Global global = engine->getGlobal();
	
	// convert string query to char * 
	string query = label->getQuery();
	strcpy( this->strQuery, query.c_str() );
	
	double lf0_msd, iw;
	
	for( k = 0; k < nOfStreams; k++ )
	{
		if( interpolationWeight != NULL)
			iw = interpolationWeight[k];
		else
			iw = 1;
		
		for( i = 0; i < nOfStates; i++ )
		{			
			if(ms.stream[k].msd_flag)
			{
                          HTS106_ModelSet_get_parameter( &ms, strQuery, label->getParsed().get_data(), this->modelMemory.stream_mean[k], this->modelMemory.stream_vari[k], 
										   &lf0_msd, k, i+2, global.parameter_iw[k] ); 
			}
			else
			{
                          HTS106_ModelSet_get_parameter( &ms, strQuery, label->getParsed().get_data(), this->modelMemory.stream_mean[k], this->modelMemory.stream_vari[k], 
										   NULL, k, i+2, global.parameter_iw[k] );
				lf0_msd = defaultMSDflag;
			}
		
			for( j = 0; j < HTS106_ModelSet_get_vector_length(&ms, k); j++ )
			{
				this->state[i].streams[k][j].mean += iw * this->modelMemory.stream_mean[k][j];
				this->state[i].streams[k][j].vari += iw * iw * this->modelMemory.stream_vari[k][j];
				this->state[i].streams[k][j].msdFlag = lf0_msd;
			}
		}
	}
	return;
}

// TODO ::
// we have to reduce this function to control 
// streams one at a time, not using if() but 
// passing the stream id as an argument 

void MAGE::Model::computeGlobalVariances( MAGE::Engine * engine, MAGE::Label * label )
{
	int i, j, k;
	
	bool gv_switch;
	
	HTS106_ModelSet ms = engine->getModelSet();
	HTS106_Global global = engine->getGlobal();
	
	// convert string query to char * 
	string query = label->getQuery();
	strcpy( this->strQuery, query.c_str() );
	
	for( k = 0; k < nOfStreams; k++ )
	{
		if( HTS106_ModelSet_use_gv( &ms, k ) )
		{
			HTS106_ModelSet_get_gv( &ms, strQuery, this->modelMemory.stream_mean[k], this->modelMemory.stream_vari[k], k, global.gv_iw[k] );
		
			for( i = 0; i < nOfStates; i++ )
			{
				for( j = 0; j < HTS106_ModelSet_get_vector_length(&ms, k); j++ )
				{
					this->state[i].gv_streams[k][j].mean = this->modelMemory.stream_mean[k][j];
					this->state[i].gv_streams[k][j].vari = this->modelMemory.stream_vari[k][j];
				}
			}
		}
	}	
	
	//TODO :: fix this
	// 1. one gv_switch per stream
	// 2. one gv_switch for nstates, not nstates gv_switch
	if( HTS106_ModelSet_have_gv_switch( &ms ) )
	{
		if( !HTS106_ModelSet_get_gv_switch( &ms, strQuery ) )
			gv_switch = false;
		else 
			gv_switch = true;
	}
	
	for( i = 0; i < nOfStates; i++ )
		for( j = 0; j < nOfStreams; j++ )
			this->state[i].gv_switch_streams[j] = false;
			
		//set manually for now
		/*this->state[i].gv_switch_streams[mgcStreamIndex] = true;
		this->state[i].gv_switch_streams[lf0StreamIndex] = true;
		this->state[i].gv_switch_streams[bapStreamIndex] = false;
		*/
		
	return;
}

void MAGE::Model::initDuration( void )
{
	for( int i = 0; i < nOfStates; i++ )
		this->state[i].duration = 0;
	
	return;
}

void MAGE::Model::initParameters( void )
{
	int i, j, k;
	
	for( k = 0; k < nOfStreams; k++ )
	{
		for( i = 0; i < nOfStates; i++ )
		{
			for( j = 0; j < maxStreamLen; j++ )
			{
				this->state[i].streams[k][j].mean = 0;
				this->state[i].streams[k][j].vari = 0;
			}
		}
	}
	return;
}

