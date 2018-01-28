/* Copyright (C) 2018  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef RHVOICE_QUESTION_MATCHER_H
#define RHVOICE_QUESTION_MATCHER_H
#ifdef __cplusplus
extern "C" {
  #endif

#define RHVOICE_PARSED_LABEL_STRING_MAX_LENGTH 1024

typedef struct
{
  char* label_string;
  short label_string_length;
  short index[128];
  short* links;
} RHVoice_parsed_label_string;

  int RHVoice_parse_label_string(const char* str,RHVoice_parsed_label_string* out);
  void RHVoice_parsed_label_string_init(RHVoice_parsed_label_string* l);
  void RHVoice_parsed_label_string_clear(RHVoice_parsed_label_string* l);
  void RHVoice_parsed_label_string_destroy(RHVoice_parsed_label_string* l);
  int RHVoice_question_match(const RHVoice_parsed_label_string* l,const char* q);
#ifdef __cplusplus
}
#endif
#endif
