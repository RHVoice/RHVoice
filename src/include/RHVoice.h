/* Copyright (C) 2010, 2011  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef RHVOICE_H
#define RHVOICE_H
#ifdef _MSC_VER
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
#else
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

  struct RHVoice_message_s;
  typedef struct RHVoice_message_s *RHVoice_message;

  typedef enum {
    RHVoice_event_word_start,
    RHVoice_event_word_end,
    RHVoice_event_sentence_start,
    RHVoice_event_sentence_end,
    RHVoice_event_mark
  } RHVoice_event_type;

  typedef struct {
    RHVoice_message message;
    RHVoice_event_type type;
    int text_position;          /* in unicode characters */
    int text_length;            /* in unicode characters */
    int audio_position;
    union {
      const char *name;
      int number;
    } id;
  } RHVoice_event;

  typedef int (*RHVoice_callback)(const short *samples,int num_samples,const RHVoice_event *events,int num_events,RHVoice_message message);

  typedef enum {
    RHVoice_position_word,
    RHVoice_position_sentence,
    RHVoice_position_mark
  } RHVoice_position_type;

  typedef struct {
    RHVoice_position_type type;
    union {
      int number;
      const char *name;
    } info;
  } RHVoice_position;

  /* Under Windows we assume that the paths are in UTF-8 */
  /* This function returns sample rate on success and 0 on error */
  int RHVoice_initialize(const char *data_path,RHVoice_callback callback,const char *cfg_path);
  void RHVoice_terminate();
  /* The following three functions accept only valid unicode strings */
  RHVoice_message RHVoice_new_message_utf8(const uint8_t *text,int length,int is_ssml);
  RHVoice_message RHVoice_new_message_utf16(const uint16_t *text,int length,int is_ssml);
  RHVoice_message RHVoice_new_message_utf32(const uint32_t *text,int length,int is_ssml);
  void RHVoice_delete_message(RHVoice_message message);
  int RHVoice_speak(RHVoice_message message);
  int RHVoice_set_position(RHVoice_message message,const RHVoice_position *position);
  int RHVoice_get_word_count(RHVoice_message message);
  int RHVoice_get_sentence_count(RHVoice_message message);
  void RHVoice_set_rate(float rate);
  float RHVoice_get_rate();
  void RHVoice_set_pitch(float pitch);
  float RHVoice_get_pitch();
  void RHVoice_set_volume(float volume);
  float RHVoice_get_volume();
  float RHVoice_get_min_rate();
  float RHVoice_get_default_rate();
  float RHVoice_get_max_rate();
  float RHVoice_get_min_pitch();
  float RHVoice_get_default_pitch();
  float RHVoice_get_max_pitch();
  float RHVoice_get_default_volume();
  float RHVoice_get_max_volume();

  typedef enum {
    RHVoice_variant_pseudo_english=1,
    RHVoice_variant_russian=2
  } RHVoice_variant;

  void RHVoice_set_variant(RHVoice_variant variant);
  RHVoice_variant RHVoice_get_variant();
  void RHVoice_set_user_data(RHVoice_message message,void *data);
  void *RHVoice_get_user_data(RHVoice_message message);

  void RHVoice_set_message_rate(RHVoice_message message,float rate);
  void RHVoice_set_message_pitch(RHVoice_message message,float pitch);
  void RHVoice_set_message_volume(RHVoice_message message,float volume);

  const char *RHVoice_get_version();

#ifdef __cplusplus
}
#endif

#endif
