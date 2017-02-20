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

 /* Some of the functions below come directly (with or without modifications) from hts-engine,		*/
 /*	which is distributed under a Modified BSD License as follow:									*/

/* ----------------------------------------------------------------- */
/*           The HMM-Based Speech Synthesis Engine "hts_engine API"  */
/*           developed by HTS Working Group                          */
/*           http://hts-engine.sourceforge.net/                      */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2001-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                                                                   */
/*                2001-2008  Tokyo Institute of Technology           */
/*                           Interdisciplinary Graduate School of    */
/*                           Science and Engineering                 */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the HTS working group nor the names of its  */
/*   contributors may be used to endorse or promote products derived */
/*   from this software without specific prior written permission.   */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */


#include "hts.h"

// constructor
MAGE::Engine::Engine()
{
}

// destructor
MAGE::Engine::~Engine()
{
	// free
	HTS106_Engine_refresh( &(this->engine) );
	
	// free memory
	HTS106_Engine_clear( &(this->engine) );
	free( this->rate_interp );
	free( this->fn_ws_mgc );
	free( this->fn_ws_lf0 );
	free( this->fn_ws_bap );
	free( this->fn_ms_mgc );
	free( this->fn_ms_lf0 );
	free( this->fn_ms_bap );
	free( this->fn_ms_dur );
	free( this->fn_ts_mgc );
	free( this->fn_ts_lf0 );
	free( this->fn_ts_bap );
	free( this->fn_ts_dur );
	free( this->fn_ms_gvm );
	free( this->fn_ms_gvl );
	free( this->fn_ms_gvf );
	free( this->fn_ts_gvm );
	free( this->fn_ts_gvl );
	free( this->fn_ts_gvf );
}

// methods
void MAGE::Engine::load( int argc, char ** argv )
{
	int i;
	double f;
	char * labfn = NULL;
	
	HTS106_File * durfp = NULL, * mgcfp = NULL, * lf0fp = NULL, * bapfp = NULL;
	HTS106_File * wavfp = NULL, * rawfp = NULL, * tracefp = NULL;
	
	// number of speakers for interpolation
	int num_interp = 0;
	
	//double * rate_interp = NULL;
	rate_interp = NULL;
	
	// HTS106_Files
	
	// number of each models for interpolation
	int num_ms_dur = 0, num_ms_mgc = 0, num_ms_lf0 = 0, num_ms_bap = 0;
	
	// number of each trees for interpolation
	int num_ts_dur = 0, num_ts_mgc = 0, num_ts_lf0 = 0, num_ts_bap = 0;
	
	int num_ws_mgc = 0, num_ws_lf0 = 0, num_ws_bap = 0;
	int num_ms_gvm = 0, num_ms_gvl = 0, num_ms_gvf = 0;
	int num_ts_gvm = 0, num_ts_gvl = 0, num_ts_gvf = 0;
	
	fn_gv_switch = NULL;
	
	// global parameter
	int sampling_rate = 16000;
	int fperiod = 80;
	double alpha = 0.42;
	int stage = 0;	// Gamma=-1/stage: if stage=0 then Gamma=0
	double beta = 0.0;
	int audio_buff_size = 1600;
	double uv_threshold = 0.5;
	double gv_weight_mgc = 1.0;
	double gv_weight_lf0 = 1.0;
	double gv_weight_bap = 1.0;
	
	double half_tone = 0.0;
	HTS106_Boolean phoneme_alignment = FALSE;
	double speech_speed = 1.0;
	HTS106_Boolean use_log_gain = FALSE;
	
	// parse command line
	if( argc == 1 )
		Usage();
	
	// delta window handler for mel-cepstrum
	fn_ws_mgc = ( char ** ) calloc( argc, sizeof( char * ) );
	// delta window handler for log f0
	fn_ws_lf0 = ( char ** ) calloc( argc, sizeof( char * ) );
	// delta window handler for band aperiodicities
	fn_ws_bap = ( char ** ) calloc( argc, sizeof( char * ) );
	
	// prepare for interpolation
	num_interp = GetNumInterp( argc, argv );
	rate_interp = ( double * ) calloc( num_interp, sizeof( double ) );
	for( i = 0; i < num_interp; i++ )
		rate_interp[i] = 1.0;
	
	fn_ms_dur = ( char ** ) calloc( num_interp, sizeof( char * ) );
	fn_ms_mgc = ( char ** ) calloc( num_interp, sizeof( char * ) );
	fn_ms_lf0 = ( char ** ) calloc( num_interp, sizeof( char * ) );
	fn_ms_bap = ( char ** ) calloc( num_interp, sizeof( char * ) );
	fn_ts_dur = ( char ** ) calloc( num_interp, sizeof( char * ) );
	fn_ts_mgc = ( char ** ) calloc( num_interp, sizeof( char * ) );
	fn_ts_lf0 = ( char ** ) calloc( num_interp, sizeof( char * ) );
	fn_ts_bap = ( char ** ) calloc( num_interp, sizeof( char * ) );
	fn_ms_gvm = ( char ** ) calloc( num_interp, sizeof( char * ) );
	fn_ms_gvl = ( char ** ) calloc( num_interp, sizeof( char * ) );
	fn_ms_gvf = ( char ** ) calloc( num_interp, sizeof( char * ) );
	fn_ts_gvm = ( char ** ) calloc( num_interp, sizeof( char * ) );
	fn_ts_gvl = ( char ** ) calloc( num_interp, sizeof( char * ) );
	fn_ts_gvf = ( char ** ) calloc( num_interp, sizeof( char * ) );
	
	// read command
	while( --argc )
	{
		if( ** ++argv == '-' )
		{
			switch( * ( * argv + 1 ) )
			{
				case 'v':
					switch( * ( * argv + 2 ) )
				{
					case 'p':
						phoneme_alignment = TRUE;
						break;
						
					default:
						Error( 1,( char * )"hts_engine: Invalid option '-v%c'.\n", * ( * argv + 2 ) );
				}
					break;
					
				case 't':
					switch( * ( * argv + 2 ) )
				{
					case 'd':
						fn_ts_dur[num_ts_dur++] = * ++argv;
						break;
						
					case 'm':
						fn_ts_mgc[num_ts_mgc++] = * ++argv;
						break;
						
					case 'f':
					case 'p':
						fn_ts_lf0[num_ts_lf0++] = * ++argv;
						break;
						
					case 'l':
						fn_ts_bap[num_ts_bap++] = * ++argv;
						break;
						
					default:
						Error( 1,( char * )"hts_engine: Invalid option '-t%c'.\n", * ( * argv + 2 ) );
				}
					--argc;
					break;
					
				case 'm':
					switch( * ( * argv + 2 ) )
				{
					case 'd':
						fn_ms_dur[num_ms_dur++] = * ++argv;
						break;
						
					case 'm':
						fn_ms_mgc[num_ms_mgc++] = * ++argv;
						break;
						
					case 'f':
					case 'p':
						fn_ms_lf0[num_ms_lf0++] = * ++argv;
						break;
						
					case 'l':
						fn_ms_bap[num_ms_bap++] = * ++argv;
						break;
						
					default:
						Error( 1,( char * )"hts_engine: Invalid option '-m%c'.\n", * ( * argv + 2 ) );
				}
					--argc;
					break;
					
				case 'd':
					switch( * ( * argv + 2 ) )
				{
					case 'm':
						fn_ws_mgc[num_ws_mgc++] = * ++argv;
						break;
						
					case 'f':
					case 'p':
						fn_ws_lf0[num_ws_lf0++] = * ++argv;
						break;
						
					case 'l':
						fn_ws_bap[num_ws_bap++] = * ++argv;
						break;
						
					default:
						Error( 1,( char * )"hts_engine: Invalid option '-d%c'.\n", * ( * argv + 2 ) );
				}
					--argc;
					break;
					
				case 'o':
					switch( * ( * argv + 2 ) )
				{
					case 'w':
						wavfp = HTS106_fopen( * ++argv, "wb" );
						break;
						
					case 'r':
						rawfp = HTS106_fopen( * ++argv, "wb" );
						break;
						
					case 'd':
						durfp = HTS106_fopen( * ++argv, "wt" );
						break;
						
					case 'm':
						mgcfp = HTS106_fopen( * ++argv, "wb" );
						break;
						
					case 'f':
					case 'p':
						lf0fp = HTS106_fopen( * ++argv, "wb" );
						break;
						
					case 'l':
						bapfp = HTS106_fopen( * ++argv, "wb" );
						break;
						
					case 't':
						tracefp = HTS106_fopen( * ++argv, "wt" );
						break;
						
					default:
						Error( 1,( char * )"hts_engine: Invalid option '-o%c'.\n", * ( * argv + 2 ) );
				}
					--argc;
					break;
					
				case 'h':
					Usage();
					break;
					
				case 's':
					sampling_rate = atoi( * ++argv );
					--argc;
					break;
					
				case 'p':
					fperiod = atoi( * ++argv );
					--argc;
					break;
					
				case 'a':
					alpha = atof( * ++argv );
					--argc;
					break;
					
				case 'g':
					stage = atoi( * ++argv );
					--argc;
					break;
					
				case 'l':
					use_log_gain = TRUE;
					break;
					
				case 'b':
					beta = atof( * ++argv );
					--argc;
					break;
					
				case 'r':
					speech_speed = atof( * ++argv );
					--argc;
					break;
					
				case 'f':
					switch( * ( * argv + 2 ) )
				{
					case 'm':
						f = atof( * ++argv );
						if( f < -24.0 )
							f = -24.0;
						if( f > 24.0 )
							f = 24.0;
						half_tone = f;
						break;
						
					default:
						Error( 1,( char * )"hts_engine: Invalid option '-f%c'.\n", * ( * argv + 2 ) );
				}
					--argc;
					break;
					
				case 'u':
					uv_threshold = atof( * ++argv );
					--argc;
					break;
					
				case 'i':
					++argv;
					argc--;
					
					for( i = 0; i < num_interp; i++ )
					{
						rate_interp[i] = atof( * ++argv );
						argc--;
					}
					break;
					
				case 'e':
					switch( * ( * argv + 2 ) )
				{
					case 'm':
						fn_ts_gvm[num_ts_gvm++] = * ++argv;
						break;
						
					case 'f':
					case 'p':
						fn_ts_gvl[num_ts_gvl++] = * ++argv;
						break;
						
					case 'l':
						fn_ts_gvf[num_ts_gvf++] = * ++argv;
						break;
						
					default:
						Error( 1,( char * )"hts_engine: Invalid option '-e%c'.\n", * ( * argv + 2 ) );
				}
					--argc;
					break;
					
				case 'c':
					switch( * ( * argv + 2 ) )
				{
					case 'm':
						fn_ms_gvm[num_ms_gvm++] = * ++argv;
						break;
						
					case 'f':
					case 'p':
						fn_ms_gvl[num_ms_gvl++] = * ++argv;
						break;
						
					case 'l':
						fn_ms_gvf[num_ms_gvf++] = * ++argv;
						break;
						
					default:
						Error( 1,( char * )"hts_engine: Invalid option '-c%c'.\n", * ( * argv + 2 ) );
				}
					--argc;
					break;
					
				case 'j':
					switch( * ( * argv + 2 ) )
				{
					case 'm':
						gv_weight_mgc = atof( * ++argv );
						break;
						
					case 'f':
					case 'p':
						gv_weight_lf0 = atof( * ++argv );
						break;
						
					case 'l':
						gv_weight_bap = atof( * ++argv );
						break;
						
					default:
						Error( 1,( char * )"hts_engine: Invalid option '-j%c'.\n", * ( * argv + 2 ) );
				}
					--argc;
					break;
					
				case 'k':
					fn_gv_switch = * ++argv;
					--argc;
					break;
					
				case 'z':
					audio_buff_size = atoi( * ++argv );
					--argc;
					break;
					
				default:
					Error( 1,( char * )"hts_engine: Invalid option '-%c'.\n", * ( * argv + 1 ) );
			}
		} 
		else 
		{
			labfn = * argv;
		}
	}
	
	// number of models,trees check
	if( num_interp != num_ts_dur || num_interp != num_ts_mgc || num_interp != num_ts_lf0 || num_interp != num_ms_dur || num_interp != num_ms_mgc || num_interp != num_ms_lf0 )
		Error( 1,( char * )"hts_engine: specify %d models( trees )for each parameter.\n", num_interp );
	
	if( num_ms_bap > 0 || num_ts_bap > 0 )
		if( num_interp != num_ms_bap || num_interp != num_ts_bap )
			Error( 1,( char * )"hts_engine: specify %d models( trees )for each parameter.\n", num_interp );
	
	// initialize( stream[0] = spectrum, stream[1] = lf0, stream[2] = band aperiodicities )
	if( num_ms_bap > 0 || num_ts_bap > 0 )
		HTS106_Engine_initialize( &engine, 3 );
	else
		HTS106_Engine_initialize( &engine, 2 );
	
	// load duration model
	HTS106_Engine_load_duration_from_fn( &engine, fn_ms_dur, fn_ts_dur, num_interp );
	
	// load stream[0]( spectrum model )
	HTS106_Engine_load_parameter_from_fn( &engine, fn_ms_mgc, fn_ts_mgc, fn_ws_mgc, 0, FALSE, num_ws_mgc, num_interp );
	
	// load stream[1]( lf0 model )
	HTS106_Engine_load_parameter_from_fn( &engine, fn_ms_lf0, fn_ts_lf0, fn_ws_lf0, 1, TRUE, num_ws_lf0, num_interp );
	
	// load stream[2]( band aperiodicities model )
	if( num_ms_bap > 0 || num_ts_bap > 0 )
		HTS106_Engine_load_parameter_from_fn( &engine, fn_ms_bap, fn_ts_bap, fn_ws_bap, 2, FALSE, num_ws_bap, num_interp );
	
	// load gv[0]( GV for spectrum )
	if( num_interp == num_ms_gvm )
	{
		if( num_ms_gvm == num_ts_gvm )
			HTS106_Engine_load_gv_from_fn( &engine, fn_ms_gvm, fn_ts_gvm, 0, num_interp );
		else
			HTS106_Engine_load_gv_from_fn( &engine, fn_ms_gvm, NULL, 0, num_interp );
	}
	
	// load gv[1]( GV for lf0 )
	if( num_interp == num_ms_gvl ){
		if( num_ms_gvl == num_ts_gvl )
			HTS106_Engine_load_gv_from_fn( &engine, fn_ms_gvl, fn_ts_gvl, 1, num_interp );
		else
			HTS106_Engine_load_gv_from_fn( &engine, fn_ms_gvl, NULL, 1, num_interp );
	}
	
	// load gv[2]( GV for band aperiodicities )
	if( num_interp == num_ms_gvf && ( num_ms_bap > 0 || num_ts_bap > 0 ) ){
		if( num_ms_gvf == num_ts_gvf )
			HTS106_Engine_load_gv_from_fn( &engine, fn_ms_gvf, fn_ts_gvf, 0, num_interp );
		else
			HTS106_Engine_load_gv_from_fn( &engine, fn_ms_gvf, NULL, 2, num_interp );
	}
	
	// load GV switch
	if( fn_gv_switch != NULL )
		HTS106_Engine_load_gv_switch_from_fn( &engine, fn_gv_switch );
	
	// set parameter
	HTS106_Engine_set_sampling_rate( &engine, sampling_rate );
	HTS106_Engine_set_fperiod( &engine, fperiod );
	HTS106_Engine_set_alpha( &engine, alpha );
	HTS106_Engine_set_gamma( &engine, stage );
	HTS106_Engine_set_log_gain( &engine, use_log_gain );
	HTS106_Engine_set_beta( &engine, beta );
	HTS106_Engine_set_audio_buff_size( &engine, audio_buff_size );
	HTS106_Engine_set_msd_threshold( &engine, 1, uv_threshold );	// set voiced/unvoiced threshold for stream[1]
	HTS106_Engine_set_gv_weight( &engine, 0, gv_weight_mgc );
	HTS106_Engine_set_gv_weight( &engine, 1, gv_weight_lf0 );
	
	if( num_ms_bap > 0 || num_ts_bap > 0 )
		HTS106_Engine_set_gv_weight( &engine, 2, gv_weight_bap );
	
	for( i = 0; i < num_interp; i++ )
	{
		HTS106_Engine_set_duration_interpolation_weight( &engine, i, rate_interp[i] );
		HTS106_Engine_set_parameter_interpolation_weight( &engine, 0, i, rate_interp[i] );
		HTS106_Engine_set_parameter_interpolation_weight( &engine, 1, i, rate_interp[i] );
		
		if( num_ms_bap > 0 || num_ts_bap > 0 )
			HTS106_Engine_set_parameter_interpolation_weight( &engine, 2, i, rate_interp[i] );
	}
	
	if( num_interp == num_ms_gvm )
		for( i = 0; i < num_interp; i++ )
			HTS106_Engine_set_gv_interpolation_weight( &engine, 0, i, rate_interp[i] );
	
	if( num_interp == num_ms_gvl )
		for( i = 0; i < num_interp; i++ )
			HTS106_Engine_set_gv_interpolation_weight( &engine, 1, i, rate_interp[i] );
	
	if( num_interp == num_ms_gvf && ( num_ms_bap > 0 || num_ts_bap > 0 ) )
		for( i = 0; i < num_interp; i++ )
			HTS106_Engine_set_gv_interpolation_weight( &engine, 2, i, rate_interp[i] );
	
	this->global = engine.global;
	this->ms = engine.ms;
	
	// close files
	if( durfp != NULL )
		HTS106_fclose( durfp );
	if( mgcfp != NULL )
		HTS106_fclose( mgcfp );
	if( lf0fp != NULL )
		HTS106_fclose( lf0fp );
	if( bapfp != NULL )
		HTS106_fclose( bapfp );
	if( wavfp != NULL )
		HTS106_fclose( wavfp );
	if( rawfp != NULL )
		HTS106_fclose( rawfp );
	if( tracefp != NULL )
		HTS106_fclose( tracefp );
	
	return;
}


// -- Engine

// Usage: output usage
void Usage( void )
{
	HTS106_show_copyright( stderr );
	fprintf( stderr, "\n" );
	fprintf( stderr, "hts_engine - The HMM-based speech synthesis engine \"hts_engine API\"\n" );
	fprintf( stderr, "\n" );
	fprintf( stderr, "	usage:\n" );
	fprintf( stderr, "		 hts_engine [ options ] [ infile ] \n" );
	fprintf( stderr, "	options:																	 [	def][ min--max]\n" );
	fprintf( stderr, "	-td tree		 : decision tree files for state duration					[	N/A]\n" );
	fprintf( stderr, "	-tm tree		 : decision tree files for spectrum						[	N/A]\n" );
	fprintf( stderr, "	-tf tree		 : decision tree files for Log F0							[	N/A]\n" );
	fprintf( stderr, "	-tl tree		 : decision tree files for band aperiodicities				 [	N/A]\n" );
	fprintf( stderr, "	-md pdf		: model files for state duration							[	N/A]\n" );
	fprintf( stderr, "	-mm pdf		: model files for spectrum								[	N/A]\n" );
	fprintf( stderr, "	-mf pdf		: model files for Log F0									[	N/A]\n" );
	fprintf( stderr, "	-ml pdf		: model files for band aperiodicities						 [	N/A]\n" );
	fprintf( stderr, "	-dm win		: window files for calculation delta of spectrum			[	N/A]\n" );
	fprintf( stderr, "	-df win		: window files for calculation delta of Log F0			[	N/A]\n" );
	fprintf( stderr, "	-dl win		: window files for calculation delta of band aperiodicities	 [	N/A]\n" );
	fprintf( stderr, "	-od s			: filename of output label with duration					[	N/A]\n" );
	fprintf( stderr, "	-om s			: filename of output spectrum							 [	N/A]\n" );
	fprintf( stderr, "	-of s			: filename of output Log F0								 [	N/A]\n" );
	fprintf( stderr, "	-ol s			: filename of output band aperiodicities						[	N/A]\n" );
	fprintf( stderr, "	-or s			: filename of output raw audio( generated speech )		 [	N/A]\n" );
	fprintf( stderr, "	-ow s			: filename of output wav audio( generated speech )		 [	N/A]\n" );
	fprintf( stderr, "	-ot s			: filename of output trace information					[	N/A]\n" );
	fprintf( stderr, "	-vp			: use phoneme alignment for duration						[	N/A]\n" );
	fprintf( stderr, "	-i	i f1 .. fi : enable interpolation & specify number( i ),coefficient( f )[	1][	 1-- ]\n" );
	fprintf( stderr, "	-s	i			: sampling frequency										[16000][	 1--48000]\n" );
	fprintf( stderr, "	-p	i			: frame period( point )									[	 80][	 1--]\n" );
	fprintf( stderr, "	-a	f			: all-pass constant										 [ 0.42][ 0.0--1.0]\n" );
	fprintf( stderr, "	-g	i			: gamma = -1 / i( if i=0 then gamma=0 )					[	0][	 0-- ]\n" );
	fprintf( stderr, "	-b	f			: postfiltering coefficient								 [	0.0][-0.8--0.8]\n" );
	fprintf( stderr, "	-l			 : regard input as log gain and output linear one( LSP )	[	N/A]\n" );
	fprintf( stderr, "	-r	f			: speech speed rate										 [	1.0][ 0.0--10.0]\n" );
	fprintf( stderr, "	-fm f			: add half-tone											 [	0.0][-24.0--24.0]\n" );
	fprintf( stderr, "	-u	f			: voiced/unvoiced threshold								 [	0.5][ 0.0--1.0]\n" );
	fprintf( stderr, "	-em tree		 : decision tree files for GV of spectrum					[	N/A]\n" );
	fprintf( stderr, "	-ef tree		 : decision tree files for GV of Log F0					[	N/A]\n" );
	fprintf( stderr, "	-el tree		 : decision tree files for GV of band aperiodicities			 [	N/A]\n" );
	fprintf( stderr, "	-cm pdf		: filenames of GV for spectrum							[	N/A]\n" );
	fprintf( stderr, "	-cf pdf		: filenames of GV for Log F0								[	N/A]\n" );
	fprintf( stderr, "	-cl pdf		: filenames of GV for band aperiodicities					 [	N/A]\n" );
	fprintf( stderr, "	-jm f			: weight of GV for spectrum								 [	1.0][ 0.0--2.0]\n" );
	fprintf( stderr, "	-jf f			: weight of GV for Log F0								 [	1.0][ 0.0--2.0]\n" );
	fprintf( stderr, "	-jl f			: weight of GV for band aperiodicities						[	1.0][ 0.0--2.0]\n" );
	fprintf( stderr, "	-k	tree		 : GV switch												 [	N/A]\n" );
	fprintf( stderr, "	-z	i			: audio buffer size										 [ 1600][	 0--48000]\n" );
	fprintf( stderr, "	infile:\n" );
	fprintf( stderr, "	label file\n" );
	fprintf( stderr, "	note:\n" );
	fprintf( stderr, "	option '-d' may be repeated to use multiple delta parameters.\n" );
	fprintf( stderr, "	generated spectrum, log F0, and band aperiodicities coefficient\n" );
	fprintf( stderr, "	sequences are saved in natural endian, binary( float )format.\n" );
	fprintf( stderr, "\n" );
	
	exit( 0 );
}

// Error: output error message
void Error( const int error, char * message, ... )
{
	va_list arg;
	
	fflush( stdout );
	fflush( stderr );
	
	if( error > 0 )
		fprintf( stderr, "\nError: " );
	else
		fprintf( stderr, "\nWarning: " );
	
	va_start( arg, message );
	vfprintf( stderr, message, arg );
	va_end( arg );
	
	fflush( stderr );
	
	if( error > 0 )
		exit( error );
	
	return;
}

// GetNumInterp: get number of speakers for interpolation from argv
int GetNumInterp( int argc, char ** argv_search )
{
	int num_interp = 1;
	while( --argc ){
		if( ** ++argv_search == '-' )
		{
			if( * ( * argv_search + 1 ) == 'i' )
			{
				num_interp = atoi( * ++argv_search );
				
				if( num_interp < 1 )
				{
					num_interp = 1;
				}
				--argc;
			}
		}
	}
	return( num_interp );
}

// -- Labels

bool isdigit_string( char * str )
{
	int i;
	
	if( sscanf( str, "%d", &i ) == 1 )
		return true;
	else
		return false;
}


// HTS106_set_duration: set duration from state duration probability distribution
double mHTS106_set_duration( int * duration, double * mean, double * vari, int size, double frame_length )
{
	int i, j;
	double temp1, temp2;
	double rho = 0.0;
	int sum = 0;
	int target_length;
	
	// if the frame length is not specified, only the mean vector is used
	if( frame_length == 0.0 )
	{
		for( i = 0; i < size; i++ )
		{
			duration[i] = ( int )( mean[i] + 0.5 );
			
			if( duration[i] < 1 )
				duration[i] = 1;
			
			sum += duration[i];
		}
		return( double )sum;
	}
	
	// get the target frame length
	target_length = ( int )( frame_length + 0.5 );
	
	// check the specified duration
	if( target_length <= size )
	{
		if( target_length < size )
			HTS106_error( -1,( char * )"HTS106_set_duration: Specified frame length is too short.\n" );
		
		for( i = 0; i < size; i++ )
			duration[i] = 1;
		
		return( double )size;
	}
	
	// RHO calculation
	temp1 = 0.0;
	temp2 = 0.0;
	for( i = 0; i < size; i++ )
	{
		temp1 += mean[i];
		temp2 += vari[i];
	}
	
	rho = ( ( double )target_length - temp1 )/ temp2;
	
	// first estimation
	for( i = 0; i < size; i++ )
	{
		duration[i] = ( int )( mean[i] + rho * vari[i] + 0.5 );
		
		if( duration[i] < 1 )
			duration[i] = 1;
		
		sum += duration[i];
	}
	
	// loop estimation
	while( target_length != sum )
	{
		// search flexible state and modify its duration
		if( target_length > sum )
		{
			j = -1;
			for( i = 0; i < size; i++ )
			{
				temp2 = abs( rho -( ( double )duration[i] + 1 - mean[i] )/ vari[i] );
				
				if( j < 0 || temp1 < temp2 )
				{
					j = i;
					temp1 = temp2;
				}
			}
			sum++;
			duration[j]++;
		}
		else 
		{
			j = -1;
			
			for( i = 0; i < size; i++ )
			{
				if( duration[i] > 1 )
				{
					temp2 = abs( rho -( ( double )duration[i] - 1 - mean[i] )/ vari[i] );
					
					if( j < 0 || temp1 < temp2 )
					{
						j = i;
						temp1 = temp2;
					}
				}
			}
			
			sum--;
			duration[j]--;
		}
	}
	return( double )target_length;
}

// HTS106_finv: calculate 1.0/variance function
double HTS106_finv( const double x )
{
	if( x >= INFTY2 )
		return 0.0;
	if( x <= -INFTY2 )
		return 0.0;
	if( x <= INVINF2 && x >= 0 )
		return INFTY;
	if( x >= -INVINF2 && x < 0 )
		return -INFTY;
	
	return( 1.0 / x );
}

// HTS106_PStream_mlpg: generate sequence of speech parameter vector maximizing its output probability for given pdf sequence
void HTS106_PStream_mlpg( HTS106_PStream * pst )
{
	int m;
	
	if( pst->length == 0 )
		return;
	
	for( m = 0; m < pst->static_length; m++ )
	{
		HTS106_PStream_calc_wuw_and_wum( pst, m );
		HTS106_PStream_ldl_factorization( pst );		// LDL factorization
		HTS106_PStream_forward_substitution( pst );	// forward substitution	
		HTS106_PStream_backward_substitution( pst, m );// backward substitution	
		
		if( pst->gv_length > 0 )
			HTS106_PStream_gv_parmgen( pst, m );
	}
	return;
}

// HTS106_PStream_calc_wuw_and_wum: calcurate W'U^{-1}W and W'U^{-1}M
void HTS106_PStream_calc_wuw_and_wum( HTS106_PStream * pst, const int m )
{
	int t, i, j, k;
	double wu;
	
	for( t = 0; t < pst->length; t++ )
	{
		// initialize
		pst->sm.wum[t] = 0.0;
		for( i = 0; i < pst->width; i++ )
			pst->sm.wuw[t][i] = 0.0;
		
		// calc WUW & WUM
		for( i = 0; i < pst->win_size; i++ )
			for( j = pst->win_l_width[i]; j <= pst->win_r_width[i]; j++ )
				if( ( t + j >= 0 ) && ( t + j < pst->length )
					&& ( pst->win_coefficient[i][-j] != 0.0 ) ){
					wu = pst->win_coefficient[i][-j] * pst->sm.ivar[t + j][i * pst->static_length + m];
					pst->sm.wum[t] += wu * pst->sm.mean[t + j][i * pst->static_length + m];
					for( k = 0;( k < pst->width ) && ( t + k < pst->length ); k++ )
						if( ( k - j <= pst->win_r_width[i] )
							&& ( pst->win_coefficient[i][k - j] != 0.0 ) ){
							pst->sm.wuw[t][k] += wu * pst->win_coefficient[i][k - j];
						}
				}
	}
	return;
}

// HTS106_PStream_ldl_factorization: Factorize W' * U^{-1} * W to L * D * L'( L: lower triangular, D: diagonal )
void HTS106_PStream_ldl_factorization( HTS106_PStream * pst )
{
	int t, i, j;
	
	for( t = 0; t < pst->length; t++ )
	{
		for( i = 1;( i < pst->width ) && ( t >= i ); i++ )
			pst->sm.wuw[t][0] -= pst->sm.wuw[t - i][i] * pst->sm.wuw[t - i][i] * pst->sm.wuw[t - i][0];
		
		for( i = 1; i < pst->width; i++ )
		{
			for( j = 1;( i + j < pst->width ) && ( t >= j ); j++ )
				pst->sm.wuw[t][i] -= pst->sm.wuw[t - j][j] * pst->sm.wuw[t - j][i + j] * pst->sm.wuw[t - j][0];
			
			pst->sm.wuw[t][i] /= pst->sm.wuw[t][0];
		}
	}
	return;
}

// HTS106_PStream_forward_substitution: forward subtitution for mlpg
void HTS106_PStream_forward_substitution( HTS106_PStream * pst )
{
	int t, i;
	
	for( t = 0; t < pst->length; t++ )
	{
		pst->sm.g[t] = pst->sm.wum[t];
		
		for( i = 1;( i < pst->width ) && ( t >= i ); i++ )
			pst->sm.g[t] -= pst->sm.wuw[t - i][i] * pst->sm.g[t - i];
	}
	return;
}

// HTS106_PStream_backward_substitution: backward subtitution for mlpg
void HTS106_PStream_backward_substitution( HTS106_PStream * pst, const int m )
{
	int t, i;
	
	for( t = pst->length - 1; t >= 0; t-- )
	{
		pst->par[t][m] = pst->sm.g[t] / pst->sm.wuw[t][0];
		
		for( i = 1;( i < pst->width ) && ( t + i < pst->length ); i++ )
			pst->par[t][m] -= pst->sm.wuw[t][i] * pst->par[t + i][m];
	}		
	return;
}

// HTS106_PStream_gv_parmgen: function for mlpg using GV
void HTS106_PStream_gv_parmgen( HTS106_PStream * pst, const int m )
{
	int t, i;
	double step = STEPINIT;
	double prev = -LZERO;
	double obj;
	
	if( pst->gv_length == 0 )
		return;
	
	HTS106_PStream_conv_gv( pst, m );
	
	if( GV_MAX_ITERATION > 0 ){
		HTS106_PStream_calc_wuw_and_wum( pst, m );
		for( i = 1; i <= GV_MAX_ITERATION; i++ )
		{
			obj = HTS106_PStream_calc_derivative( pst, m );
			
			if( obj > prev )
				step *= STEPDEC;
			
			if( obj < prev )
				step *= STEPINC;
			
			for( t = 0; t < pst->length; t++ )
				pst->par[t][m] += step * pst->sm.g[t];
			
			prev = obj;
		}
	}
	return;
}

// HTS106_PStream_conv_gv: subfunction for mlpg using GV
void HTS106_PStream_conv_gv( HTS106_PStream * pst, const int m )
{
	int t;
	double ratio;
	double mean;
	double vari;
	
	HTS106_PStream_calc_gv( pst, m, &mean, &vari );
	
	ratio = sqrt( pst->gv_mean[m] / vari );
	
	for( t = 0; t < pst->length; t++ )
		if( pst->gv_switch[t] )
			pst->par[t][m] = ratio * ( pst->par[t][m] - mean )+ mean;
	return;
}

// HTS106_PStream_calc_derivative: subfunction for mlpg using GV
double HTS106_PStream_calc_derivative( HTS106_PStream * pst, const int m )
{
	int t, i;
	double mean;
	double vari;
	double dv;
	double h;
	double gvobj;
	double hmmobj;
	const double w = 1.0 /( pst->win_size * pst->length );
	
	HTS106_PStream_calc_gv( pst, m, &mean, &vari );
	gvobj = -0.5 * W2 * vari * pst->gv_vari[m] * ( vari - 2.0 * pst->gv_mean[m] );
	dv = -2.0 * pst->gv_vari[m] * ( vari - pst->gv_mean[m] )/ pst->length;
	
	for( t = 0; t < pst->length; t++ )
	{
		pst->sm.g[t] = pst->sm.wuw[t][0] * pst->par[t][m];
		
		for( i = 1; i < pst->width; i++ )
		{
			if( t + i < pst->length )
				pst->sm.g[t] += pst->sm.wuw[t][i] * pst->par[t + i][m];
			
			if( t + 1 > i )
				pst->sm.g[t] += pst->sm.wuw[t - i][i] * pst->par[t - i][m];
		}
	}
	
	for( t = 0, hmmobj = 0.0; t < pst->length; t++ )
	{
		hmmobj += W1 * w * pst->par[t][m] * ( pst->sm.wum[t] - 0.5 * pst->sm.g[t] );
		h = - W1 * w * pst->sm.wuw[t][1 - 1] - W2 * 2.0 /( pst->length * pst->length ) * ( ( pst->length - 1 ) * pst->gv_vari[m] * ( vari - pst->gv_mean[m] )+ 2.0 * pst->gv_vari[m] * ( pst->par[t][m] - mean ) * ( pst->par[t][m] - mean ) );
		
		if( pst->gv_switch[t] )
			pst->sm.g[t] = 1.0 / h * ( W1 * w * ( -pst->sm.g[t] + pst->sm.wum[t] )+ W2 * dv * ( pst->par[t][m] - mean ) );
		else
			pst->sm.g[t] = 1.0 / h * ( W1 * w * ( -pst->sm.g[t] + pst->sm.wum[t] ) );
	}
	return( -( hmmobj + gvobj ) );
}

// HTS106_PStream_calc_gv: subfunction for mlpg using GV
void HTS106_PStream_calc_gv( HTS106_PStream * pst, const int m, double * mean, double * vari )
{
	int t;
	
	 * mean = 0.0;
	
	for( t = 0; t < pst->length; t++ )
		if( pst->gv_switch[t] )
			 * mean += pst->par[t][m];
	
	 * mean /= pst->gv_length;
	
	 * vari = 0.0;
	
	for( t = 0; t < pst->length; t++ )
		if( pst->gv_switch[t] )
			 * vari += ( pst->par[t][m] - * mean ) * ( pst->par[t][m] - * mean );
	
	 * vari /= pst->gv_length;
	return;
}

