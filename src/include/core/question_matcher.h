/* Copyright (C) 2018  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 2.1 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */

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
  int RHVoice_parsed_label_string_copy(const RHVoice_parsed_label_string* from,RHVoice_parsed_label_string* to);
  int RHVoice_question_match(const RHVoice_parsed_label_string* l,const char* q);
#ifdef __cplusplus
}
#endif
#endif

#ifndef RHVOICE_QUESTION_MATCHER_HPP
#define RHVOICE_QUESTION_MATCHER_HPP

#ifdef __cplusplus

namespace RHVoice
{
  struct parsed_label_string
  {
    parsed_label_string()
    {
      init();
}

    parsed_label_string(const parsed_label_string& other)
    {
      init();
      copy(other);
}

    parsed_label_string& operator=(const parsed_label_string& other)
    {
      copy(other);
      return *this;
}

    ~parsed_label_string()
    {
      clear();
}

    const RHVoice_parsed_label_string* get_data() const
    {
      return &data;
}

    void parse(const char* s);

    bool match(const char* q) const
    {
      return (RHVoice_question_match(&data,q)!=0);
}

  private:

    void init()
    {
      RHVoice_parsed_label_string_init(&data);
}

    void clear()
    {
      RHVoice_parsed_label_string_clear(&data);
    }

    void copy(const parsed_label_string& other);

    RHVoice_parsed_label_string data;
  };
}
#endif
#endif
