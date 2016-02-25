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

#include <fstream>
#include <map>

#include "Constants.h"
#include "MathFunctions.h"

#include "Frame.h"
#include "Label.h"
#include "Vocoder.h"

#include "State.h"
#include "Model.h"
#include "Distribution.h"

#include "MemQueue.h"
#include "LabelQueue.h"
#include "ModelQueue.h"
#include "FrameQueue.h"

namespace MAGE 
{
	/** 
	 *  \brief		The implemented Mage API for Performative HMM-based Speech & Singing Synthesis.
	 *  \details	This class is used to define the Mage API to be used in various platforms and applications.
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
	class Mage 
	{
		public:
		
			/**
			 *	Constructor that allocates the required memory for a Mage instance and initializes 
			 *	all the parameters in default values.
			 */		
			Mage( void );
		
			/**
			 *	Constructor that allocates the required memory for a Mage instance and initializes 
			 *	all the parameters in values passed from command line arguments.
			 *
			 *	@param EngineName The name of the engine to be added.
			 *	@param argc The number of arguments passed.
			 *	@param argv The actual arguments passed to be used for the initialization .
			 */	
			Mage( std::string EngineName, int argc, char ** argv );
		
			/**
			 *	Constructor that allocates the required memory for a Mage instance and initializes 
			 *	all the parameters in values passed from a configuration file.
			 *
			 *	@param EngineName The name of the engine to be added.
			 *	@param confFilename The path and name of the configuration file.
			 */			
			Mage( std::string EngineName, std::string confFilename );
			
			/**
			 *	Destructor that disallocates all the memory used from a Mage instance.
			 */	
			~Mage( void );
		
// getters
		
			/**
			 *	This function gets the currently used Frame.
			 *
			 *	@return The currently used Frame.
			 */
			inline Frame getFrame( void ){ return( this->frame ); };
		
			/**
			 *	This function gets the currently used Label.
			 *
			 *	@return The currently used Label.
			 */
			inline Label getLabel( void ){ return( this->label ); };
		
			/**
			 *	This function gets the currently used Model.
			 *
			 *	@return The currently used Model.
			 */
			inline Model * getModel( void ){ return( this->model ); };
		
			/**
			 *	This function gets the currently used Vocoder.
			 *
			 *	@return The currently used Vocoder.
			 */
			inline Vocoder * getVocoder( void ){ return( this->vocoder ); };

			/**
			 *	This function gets the currently used Label ringbuffer (LabelQueue).
			 *
			 *	@return The currently used LabelQueue.
			 */
			inline LabelQueue * getLabelQueue( void ){ return( this->labelQueue ); };
		
			/**
			 *	This function gets the currently used Model ringbuffer (ModelQueue).
			 *
			 *	@return The currently used ModelQueue.
			 */
			inline ModelQueue * getModelQueue( void ){ return( this->modelQueue ); };
		
			/**
			 *	This function gets the currently used Frame ringbuffer (FrameQueue).
			 *
			 *	@return The currently used FrameQueue.
			 */
			inline FrameQueue * getFrameQueue( void ){ return( this->frameQueue ); };

			/**
			 *	This function gets the current index / number of samples.
			 *
			 *	@return The current index / number of samples.
			 */
			inline int getSampleCounter( void ){ return( this->sampleCount ); };

			/**
			 *	This function gets the current speed used in the Vocoder.
			 *
			 *	@return The speed value used in the Vocoder.
			 */
			inline double getSpeed( void ){ return( this->hopLen ); };
		
			/**
			 *	This function gets the speed used in the pop()'d Label.
			 *
			 *	@return The speed value used in the pop()'d Label.
			 */
			inline double getLabelSpeed( void ){ return( this->labelSpeed ); };
		
			/**
			 *	This function returns true if the currently added Engine instance is ready to be accessed, and false otherwise. 
			 *
			 *	@return True if the currently added Engine instance is ready to be accessed, and false otherwise. 
			 */
			inline bool ready( void ){ return( this->flagReady ); };
			
			/**
			 *	This function gets the name of the default engine used.
			 *
			 *	@return The name of the default engine used.
			 */
			inline std::string getDefaultEngine( void ){ return( this->defaultEngine ); };		
		
			/**
			 *	This function gets the pitch value used in the Vocoder.
			 *
			 *	@return The pitch value used in the Vocoder.
			 */
			double getPitch( void );
		
			/**
			 *	This function gets the alpha value used in the Vocoder.
			 *
			 *	@return The alpha value used in the Vocoder.
			 */		
			double getAlpha( void );
		
			/**
			 *	This function gets the gamma value used in the Vocoder.
			 *
			 *	@return The gamma value used in the Vocoder.
			 */
			double getGamma( void );
		
			/**
			 *	This function gets the pade order value used in the Vocoder.
			 *
			 *	@return The pade order value used in the Vocoder.
			 */
			double getPOrder( void );
		
			/**
			 *	This function gets the volume value used in the Vocoder.
			 *
			 *	@return The volume value used in the Vocoder.
			 */
			double getVolume( void );
		
			/**
			 *	This function gets the duration value of the Model used.
			 *
			 *	@return The duration value of the Model used.
			 */
			double getDuration( void );

//setters
		
			/**
			 *	This function sets a new Frame instance instead of the currently used.
			 *
			 *	@param frame The new Frame instance.
			 */	
			inline void setFrame( Frame frame ){ this->frame = frame; };
		
			/**
			 *	This function sets a new Label instance instead of the currently used.
			 *
			 *	@param label The new Label instance.
			 */	
			inline void setLabel( Label label ){ this->label = label; };
		
			/**
			 *	This function sets a new Model instance instead of the currently used.
			 *
			 *	@param model The new Model instance.
			 */	
			inline void setModel( Model * model ){ this->model = model; };
		
			/**
			 *	This function sets a new Vocoder instance instead of the currently used.
			 *
			 *	@param vocoder The new Vocoder instance.
			 */	
			inline void setVocoder( Vocoder * vocoder ){ this->vocoder = vocoder; };
		
			/**
			 *	This function sets a new speed value in the pop()'d Labels. This function is less reactive
			 *	than setSpeed (it has a delay of nOfLookup Model instances / labels).
			 *
			 *	@param labelSpeed The new Vocoder instance.
			 */	
			inline void setLabelSpeed( double labelSpeed ){ this->labelSpeed = labelSpeed; }; // less reactive
		
			/**
			 *	This function sets a new LabelQueue instance instead of the currently used.
			 *
			 *	@param labelQueue The new LabelQueue instance.
			 */	
			inline void setLabelQueue( LabelQueue * labelQueue ){ this->labelQueue = labelQueue; };
		
			/**
			 *	This function sets a new ModelQueue instance instead of the currently used.
			 *
			 *	@param modelQueue The new ModelQueue instance.
			 */	
			inline void setModelQueue( ModelQueue * modelQueue ){ this->modelQueue = modelQueue; };
		
			/**
			 *	This function sets a new FrameQueue instance instead of the currently used.
			 *
			 *	@param frameQueue The new FrameQueue instance.
			 */	
			inline void setFrameQueue( FrameQueue * frameQueue ){ this->frameQueue = frameQueue; };
		
			/**
			 *	This function sets a new alpha value to be used in the Vocoder.
			 *
			 *	@param alpha The new alpha value.
			 */	
			void setAlpha( double alpha );
		
			/**
			 *	This function sets a new gamma value to be used in the Vocoder.
			 *
			 *	@param gamma The new gamma value.
			 */	
			void setGamma( double gamma );
		
			/**
			 *	This function sets a new pade order value to be used in the Vocoder.
			 *
			 *	@param pd The new pade order value.
			 */	
			void setPOrder( double pd );
		
			/**
			 *	This function sets a new volume value to be used in the Vocoder.
			 *
			 *	@param volume The new volume value.
			 */	
			void setVolume( double volume );
		
			/**
			 *	This function sets a new pitch value to be used in the Vocoder, taking into account a  
			 *	given action. The existing pitch value can  be :: replaced / overwritten, shifted, scaled,  
			 *	or decide to take no action or use the pitch trajectory generated by the Model.
			 *	Current values for action are -1, 0, 1, 2 and 3. See Constant.h for details.
			 *
			 *	@param pitch The new pitch value.
			 *	@param action The action to be taken over the existing pitch value.
			 */
			void setPitch( double pitch, int action );
		
			/**
			 *	This function sets a new speed value to be used in the Vocoder, taking into account a  
			 *	given action. The existing speed value can  be :: replaced / overwritten, shifted, scaled,  
			 *	or decide to take no action or use the default speed. 
			 *	Current values for action are -1, 0, 1, 2 and 3. See Constant.h for details.
			 *
			 *	@param speed The new speed value.
			 *	@param action The action to be taken over the existing speed value.
			 */
			void setSpeed( double speed, int action );

			/**
			 *	This function sets a new set of duration values for every State of the current Model.
			 *	The existing set of duration values can be :: replaced / overwritten, shifted, scaled,  
			 *	or decide to take no action or use the default State Model durations. 
			 *
			 *	@param updateFunction The new set of duration values.
			 *	@param action The action to be taken over the existing speed value.
			 */		
			void setDuration( double * updateFunction, int action );
		
			/**
			 *	This function sets a different existing engine as the default to be used.
			 *
			 *	@param defaultEngine The name of new engine to be set as the default.
			 */	
			void setDefaultEngine( std::string defaultEngine );
		
			/**
			 *	This function sets a different set of interpolation weights for every stream of a given engine used.
			 *
			 *	@param interpolationFunctionsSet The name of the engine and the set of interpolation weights to be set.
			 */	
			void setInterpolationFunctions( std::map < std::string, double * > interpolationFunctionsSet ); // interpolation function 
																					// for the duration & parameter (streams)  models
		
// methods

			/** 
			 *	This function returns a single sample from the Vocoder.
			 * 
			 *	@return One sample from the Vocoder given current user controls over the Model 
			 *			the instances spectral coefficients, the fundamental frequency, etc.
			 */
			double popSamples( void );

			/**
			 *	This function removes the oldest Label instance from the queue. 
			 * 
			 *	Note that once pop() has been called, the Label item can be overwritten at any time
			 *	by a subsequent next()/push(). 
			 *
			 *	@return True if the Label was pop()'d, otherwise false.
			 */
			bool popLabel( void );

			/**
			 *	This function runs all the Mage controls until the speech sample generation. 
			 * 
			 */
			void run( void );	
		
			/**
			 *	This function resets Mage to its default values, discarding all user controls. 
			 * 
			 */
			void reset( void );
		
			/**
			 *	This function resets the Vocoder of Mage to its default values, discarding all user controls. 
			 * 
			 */
			void resetVocoder( void );
		
			/**
			 *	This function initializes and controls the Model queue of Mage as well as the initial 
			 *	interpolation weights passed to the Engine by command line or a configuration file. 
			 * 
			 */
			void prepareModel( void );
		
			/**
			 *	This function generates the speech samples. 
			 * 
			 */
			void updateSamples( void );	
		
			/**
			 *	This function computes the durations from the Model by taking also into account 
			 *	the user control over the passed durations. 
			 * 
			 */
			void computeDuration( void );
		
			/**
			 *	This function computes the parameters for every coefficients stream. 
			 * 
			 */
			void computeParameters( void );
		
			/**
			 *	This function optimizes the generated parameters for every coefficients stream. 
			 * 
			 */
			void optimizeParameters( void );

			/**
			 *	This function adds a new Label instance in the queue. 
			 *
			 *	@param label The new Label instance to be added in the LabelQueue. 
			 */
			void pushLabel( Label label );
		
			/**
			 *	This function creates and adds a new Engine instance in the map of Engine instances. 
			 *
			 *	@param EngineName The name of the new Engine instance. 
			 *	@param argc The number of the command line arguments needed to create and initialize 
			 *					the new Engine instance. 
			 *	@param argv The command line arguments needed to create and initialize the new Engine instance. 
			 */
			void addEngine( std::string EngineName, int argc, char ** argv );
		
			/**
			 *	This function creates and adds a new Engine instance in the map of Engine instances. 
			 *
			 *	@param EngineName The name of the new Engine instance. 
			 *	@param confFilename The configuration file containing all the arguments needed to create and 
			 *							initialize the new Engine instance. 
			 */
			void addEngine( std::string EngineName, std::string confFilename );

			/**
			 *	This function removes and disallocates a given Engine instance from the map of Engine instances. 
			 *
			 *	@param EngineName The name of the Engine instance to be removed. 
			 */	
			void removeEngine( std::string EngineName );
		
			/**
			 *	This function enables or disables the interpolation between all the added Engine instance in the map. 
			 *
			 *	@param interpolationFlag True in order to enable the interpolation and false to disable it. 
			 */	
			inline void enableInterpolation( bool interpolationFlag ){ this->interpolationFlag = interpolationFlag; };   // enable interpolation 
																								// for the duration & parameter (streams) models
		
			/**
			 *	This function prints the current interpolation weights for every Engine instance in the map. 
			 *
			 */
			void printInterpolationWeights( void );
			
			/**
			 *	This function resets the current interpolation weights for every Engine instance in the map to zero. 
			 *
			 */
			void resetInterpolationWeights( void );
		
			/**
			 *	This function checks if the currently added Engine instance is initialized and ready to be used. 
			 *
			 */
			void checkReady( void );
			
			/**
			 *	This function returns the compilation time and day.
			 *
			 *	@return The compilation time and day.
			 */
			std::string timestamp( void );

		protected:		
		
// --- Queues ---	
		
			/** 
			 *	\var LabelQueue labelQueue.
			 *	\brief It contains the queue of Label instances used from Mage.
			 */
			LabelQueue * labelQueue;
		
			/** 
			 *	\var ModelQueue modelQueue.
			 *	\brief It contains the queue of Model instances used from Mage.
			 */
			ModelQueue * modelQueue;
		
			/** 
			 *	\var FrameQueue frameQueue.
			 *	\brief It contains the queue of Frame instances used from Mage.
			 */
			FrameQueue * frameQueue;
		
// --- HTS Engine & Voices (Engine) ---
		
			/** 
			 *	\var string defaultEngine.
			 *	\brief It contains the name of the default Engine instance used from Mage.
			 */
			string defaultEngine;
		
			/** 
			 *	\var bool interpolationFlag.
			 *	\brief It contains the information whether or not the interpolation between 
			 *	the available Engine instances is enabled.
			 */
			bool interpolationFlag;
		
			/** 
			 *	\var std::map < std::string, std::pair < double * , Engine * > > engine.
			 *	\brief It contains every Engine instance with the corresponding interpolationFunctions used from Mage. Each Engine is associated to a unique string key.
			 */		
			std::map < std::string, std::pair < double * , Engine * > > engine; // Format :: 
													// [ "engineName", interpolationWeightsOfStreams[], HTS Engine ]
													// interpolationWeights for number of streams + duration 		
// --- Model ---
		
			/** 
			 *	\var Model model.
			 *	\brief It contains the Model instance currently used.
			 */
			Model * model;
		
// --- SPTK vocoder ---
		
			/** 
			 *	\var int hopLen.
			 *	\brief It contains the speed passed to the Vocoder.
			 */
			int hopLen;
		
			/** 
			*	\var int sampleCount.
			*	\brief It contains the number of samples generated.
			*/
			int sampleCount;
		
			/** 
			 *	\var Vocoder vocoder.
			 *	\brief It contains the Vocoder instance currently used.
			 */
			Vocoder * vocoder;

// --- Frame ---
		
			/** 
			 *	\var Frame frame.
			 *	\brief It contains the Frame instance currently used.
			 */
			Frame frame;
		
// --- Label ---
		
			/** 
			 *	\var Label label.
			 *	\brief It contains the Label instance currently used.
			 */
			Label label;
		
			/** 
			 *	\var double labelSpeed.
			 *	\brief It contains the speed of the currently used Label instance.
			 */
			double labelSpeed;	// we need this because the speed changes on the lable 
								// level and we need to have memory of this

		private:

			bool flag;
			int  action;
		
			double updateFunction[nOfStates]; // replace with void * function
			double interpolationWeights[nOfStreams + 1];
		
// HTS106_Engine & Voices (Engine)
			int argc;
			char ** argv;
			bool flagReady;

// methods
			void init( void );
		
			void addEngine( std::string name ); // called by addEngine( string, ...)
			void parseConfigFile( std::string filename );
			/**
			 * free memory of this->argv allocated in parseConfigFile
			 * @return 
			 */
			void freeArgv( void );
			void optimizeParameters( Engine * engine );
			void checkInterpolationFunctions( void );
	};
} // namespace
