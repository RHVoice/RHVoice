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

#include <stdint.h>

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
    RHVoice_event_mark,
    RHVoice_event_message_start,
    RHVoice_event_message_end
  } RHVoice_event_type;

  typedef struct {
    const RHVoice_message message;
    RHVoice_event_type type;
    int text_position;          /* in unicode characters */
    int text_length;            /* in unicode characters */
    int audio_position;
    union {
      const char *name;
      int number;
    } id;
  } RHVoice_event;

  typedef int (*RHVoice_callback)(const short *samples,int num_samples,const RHVoice_event *events,int num_events);

  /* This function returns sample rate on success and 0 on error */
  int RHVoice_initialize(const char *path,RHVoice_callback callback);
  void RHVoice_terminate();
  /* The following three functions accept only valid unicode strings */
  RHVoice_message RHVoice_new_message_utf8(const uint8_t *text,int length,int is_ssml);
  RHVoice_message RHVoice_new_message_utf16(const uint16_t *text,int length,int is_ssml);
  RHVoice_message RHVoice_new_message_utf32(const uint32_t *text,int length,int is_ssml);
  void RHVoice_delete_message(RHVoice_message message);
  int RHVoice_speak(RHVoice_message message);
  /* int RHVoice_load_user_dict(const char *path); */
  void RHVoice_set_rate(unsigned int rate);
  unsigned int RHVoice_get_rate();
  void RHVoice_set_pitch(unsigned int pitch);
  unsigned int RHVoice_get_pitch();
  void RHVoice_set_volume(unsigned int volume);
  unsigned int RHVoice_get_volume();
  const char *RHVoice_get_version();

#ifdef __cplusplus
}
#endif

#endif
