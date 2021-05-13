/* Copyright (C) 2012, 2014, 2020  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 2.1 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the structied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#ifndef RHVOICE_COMMON_H
#define RHVOICE_COMMON_H

typedef enum {
  RHVoice_voice_gender_unknown,
  RHVoice_voice_gender_male,
  RHVoice_voice_gender_female
} RHVoice_voice_gender;

typedef enum {
  RHVoice_punctuation_default,
  RHVoice_punctuation_none,
  RHVoice_punctuation_all,
  RHVoice_punctuation_some
} RHVoice_punctuation_mode;

typedef enum {
  RHVoice_capitals_default,
  RHVoice_capitals_off,
  RHVoice_capitals_word,
  RHVoice_capitals_pitch,
  RHVoice_capitals_sound
} RHVoice_capitals_mode;

typedef enum
  {
    RHVoice_log_level_trace,
    RHVoice_log_level_debug,
    RHVoice_log_level_info,
    RHVoice_log_level_warning,
    RHVoice_log_level_error
  } RHVoice_log_level;

typedef enum {
              RHVoice_synth_flag_dont_clip_rate=1
} RHVoice_synth_flag;
#endif
