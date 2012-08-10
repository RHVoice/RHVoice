/* Copyright (C) 2010, 2011, 2012  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the structied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef RHVOICE_H
#define RHVOICE_H

#include "RHVoice_common.h"

#ifdef __cplusplus
extern "C" {
#else
#include <stddef.h>
#endif

  struct RHVoice_tts_engine_struct;
  typedef struct RHVoice_tts_engine_struct* RHVoice_tts_engine;

typedef struct
{
  /* This is the only function the caller is *required* to provide. */
  /* Return 0 to stop synthesis. */
  int (*play_speech)(const short* samples,unsigned int count,void* user_data);
  /* These functions are optional, */
  /* but please make sure to set unused function pointers to 0. */
  int (*process_mark)(const char* name,void* user_data);
  int (*word_starts)(unsigned int position,unsigned int length,void* user_data);
  int (*word_ends)(unsigned int position,unsigned int length,void* user_data);
  int (*sentence_starts)(unsigned int position,unsigned int length,void* user_data);
  int (*sentence_ends)(unsigned int position,unsigned int length,void* user_data);
  int(*play_audio)(const char* src,void *user_data);
} RHVoice_callbacks;

  typedef enum {
    RHVoice_preload_voices=1
  } RHVoice_init_option;
  typedef unsigned int RHVoice_init_options;

  typedef struct
  {
    /* The paths should be encoded as utf-8 strings. */
    const char *data_path,*config_path;
    RHVoice_callbacks callbacks;
    RHVoice_init_options options;
  } RHVoice_init_params;

  typedef enum {
    RHVoice_message_text,
    RHVoice_message_ssml,
    RHVoice_message_characters
  } RHVoice_message_type;

  struct RHVoice_message_struct;
  typedef struct RHVoice_message_struct* RHVoice_message;

  typedef struct
  {
    /* Language code. */
    const char* language;
    const char* name;
    RHVoice_voice_gender gender;
  } RHVoice_voice_info;

  typedef struct
  {
    /* The main voice must not be 0, but the extra voice is optional. */
    /* If the extra voice is defined, */
    /* the automatic language switching will be enabled. */
    /* Use the information about the compatible languages to form pairs of voices. */
    /* Only languages without common letters can be used this way. */
    /* If RHVoice is unable to determine the language of the sentence, e.g. it is just one number, */
    /* the synthesizer will speak it with the main voice. */
    const char* main_voice,*extra_voice;
    /* The values must be between -1 and 1. */
    /*     They are normalized this way, because users can set different */
    /* parameters for different voices in the configuration file. */
    double absolute_rate,absolute_pitch,absolute_volume;
    /* Relative values, in case someone needs them. */
    /* If you don't, just set each of them to 1. */
    double relative_rate,relative_pitch,relative_volume;
  } RHVoice_synth_params;

  const char* RHVoice_get_version();

  RHVoice_tts_engine RHVoice_new_tts_engine(const RHVoice_init_params* init_params);
  void RHVoice_delete_tts_engine(RHVoice_tts_engine tts_engine);

  unsigned int RHVoice_get_number_of_voices(RHVoice_tts_engine tts_engine);
  const RHVoice_voice_info* RHVoice_get_voices(RHVoice_tts_engine tts_engine);
  int RHVoice_are_languages_compatible(RHVoice_tts_engine tts_engine,const char* language1,const char* language2);

  /* Text should be a valid utf-8 string */
  RHVoice_message RHVoice_new_message(RHVoice_tts_engine tts_engine,const char* text,unsigned int length,RHVoice_message_type message_type,const RHVoice_synth_params* synth_params,void* user_data);

  /* On Windows the library is now built with MSVC instead of Mingw, */
  /* so wchar_t will always mean utf-16 there */
  RHVoice_message RHVoice_new_message_w(RHVoice_tts_engine tts_engine,const wchar_t* text,unsigned int length,RHVoice_message_type message_type,const RHVoice_synth_params* synth_params,void* user_data);

  void RHVoice_delete_message(RHVoice_message message);

  int RHVoice_speak(RHVoice_message message);

#ifdef __cplusplus
}
#endif
#endif
