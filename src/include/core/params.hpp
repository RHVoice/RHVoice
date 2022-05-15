/* Copyright (C) 2012, 2014  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_PARAMS_HPP
#define RHVOICE_PARAMS_HPP

#include "config.hpp"
#include "property.hpp"
#include "RHVoice_common.h"

namespace RHVoice
{
  class voice_params
  {
  public:
    numeric_property<double> default_rate,min_rate,max_rate;
    numeric_property<double> default_pitch,min_pitch,max_pitch;
    numeric_property<double> default_volume,min_volume,max_volume;
    numeric_property<double> cap_pitch_factor;
    numeric_property<double> min_sonic_rate;

    voice_params();
    void register_self(config& cfg,const std::string& prefix=std::string());
    void default_to(const voice_params& other);

  private:
    voice_params(const voice_params& other);
    voice_params& operator=(const voice_params&);
  };

  class text_params
  {
  public:
    char_property stress_marker;

    text_params();
    void register_self(config& cfg,const std::string& prefix=std::string());
    void default_to(const text_params& other);

  private:
    text_params(const text_params&);
    text_params& operator=(const text_params&);
  };

  class absolute_speech_params
  {
  public:
    numeric_property<double> rate,pitch,volume;
    absolute_speech_params();
    void default_to(const absolute_speech_params& other);

  private:
    absolute_speech_params(const absolute_speech_params& other);
    absolute_speech_params& operator=(const absolute_speech_params& other);
  };

  class relative_speech_params
  {
  public:
    numeric_property<double> rate,pitch,volume;
    relative_speech_params();
    void default_to(const relative_speech_params& other);

  private:
    relative_speech_params(const relative_speech_params& other);
    relative_speech_params& operator=(const relative_speech_params& other);
  };

  struct speech_params
  {
    absolute_speech_params absolute;
    relative_speech_params relative;
  };

  class verbosity_params
  {
  public:
    c_enum_property<RHVoice_punctuation_mode> punctuation_mode;
    charset_property punctuation_list;
    c_enum_property<RHVoice_capitals_mode> capitals_mode;

    verbosity_params();
    void register_self(config& cfg,const std::string& prefix=std::string());
    void default_to(const verbosity_params& other);

  private:
    verbosity_params(const verbosity_params&);
    verbosity_params& operator=(const verbosity_params&);
  };

  struct stream_params
  {
    numeric_property<unsigned int> fixed_size{"stream.fixed_size", 1, 1, 10};
    numeric_property<unsigned int> view_size{"stream.view_size", 3, 1, 10};

    void register_self(config& cfg);
  };
}
#endif
