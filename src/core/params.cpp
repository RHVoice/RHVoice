/* Copyright (C) 2012, 2014, 2018, 2020  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include "core/params.hpp"

namespace
{
  const double MAX_MAX_VOLUME=4;
  const double MAX_MAX_RATE=5;
  const double NO_MIN_SONIC_RATE=MAX_MAX_RATE+1;
}

#ifndef MAX_VOLUME
#define MAX_VOLUME 2
#endif

#ifndef MAX_RATE
#define MAX_RATE 2
#endif

#ifndef MIN_SONIC_RATE
#define MIN_SONIC_RATE 1
#endif

#ifndef DEFAULT_PUNCTUATION_MODE
#define DEFAULT_PUNCTUATION_MODE RHVoice_punctuation_none
#endif

namespace RHVoice
{
  voice_params::voice_params():
    default_rate("default_rate",1,0.2,MAX_MAX_RATE),
    min_rate("min_rate",0.5,0.2,1),
    max_rate("max_rate",MAX_RATE,1,MAX_MAX_RATE),
    default_pitch("default_pitch",1,0.5,2),
    min_pitch("min_pitch",0.5,0.5,1),
    max_pitch("max_pitch",2,1,2),
    default_volume("default_volume",1,0.25,MAX_MAX_VOLUME),
    min_volume("min_volume",0.25,0.25,1),
    max_volume("max_volume",MAX_VOLUME,1,MAX_MAX_VOLUME),
    cap_pitch_factor("cap_pitch_factor",1.3,0.5,2),
    min_sonic_rate("min_sonic_rate",MIN_SONIC_RATE,0.2,NO_MIN_SONIC_RATE)
  {
  }

  void voice_params::default_to(const voice_params& other)
  {
    default_rate.default_to(other.default_rate);
    min_rate.default_to(other.min_rate);
    max_rate.default_to(other.max_rate);
    default_pitch.default_to(other.default_pitch);
    min_pitch.default_to(other.min_pitch);
    max_pitch.default_to(other.max_pitch);
    default_volume.default_to(other.default_volume);
    min_volume.default_to(other.min_volume);
    max_volume.default_to(other.max_volume);
    cap_pitch_factor.default_to(other.cap_pitch_factor);
    min_sonic_rate.default_to(other.min_sonic_rate);
  }

  void voice_params::register_self(config& cfg,const std::string& prefix)
  {
    cfg.register_setting(default_rate,prefix);
    cfg.register_setting(min_rate,prefix);
    cfg.register_setting(max_rate,prefix);
    cfg.register_setting(default_pitch,prefix);
    cfg.register_setting(min_pitch,prefix);
    cfg.register_setting(max_pitch,prefix);
    cfg.register_setting(default_volume,prefix);
    cfg.register_setting(min_volume,prefix);
    cfg.register_setting(max_volume,prefix);
    cfg.register_setting(cap_pitch_factor,prefix);
    cfg.register_setting(min_sonic_rate,prefix);
  }

  text_params::text_params():
    stress_marker("stress_marker",'+')
  {
  }

  void text_params::default_to(const text_params& other)
  {
    stress_marker.default_to(other.stress_marker);
  }

  void text_params::register_self(config& cfg,const std::string& prefix)
  {
    cfg.register_setting(stress_marker,prefix);
  }

  absolute_speech_params::absolute_speech_params():
    rate("rate",0,-1,1),
    pitch("pitch",0,-1,1),
    volume("volume",0,-1,1)
  {
  }

  void absolute_speech_params::default_to(const absolute_speech_params& other)
  {
    rate.default_to(other.rate);
    pitch.default_to(other.pitch);
    volume.default_to(other.volume);
  }

  relative_speech_params::relative_speech_params():
    rate("rate",1,0.1,10),
    pitch("pitch",1,0.5,2),
    volume("volume",1,0.25,2)
  {
  }

  void relative_speech_params::default_to(const relative_speech_params& other)
  {
    rate.default_to(other.rate);
    pitch.default_to(other.pitch);
    volume.default_to(other.volume);
  }

  verbosity_params::verbosity_params():
    punctuation_mode("punctuation_mode",DEFAULT_PUNCTUATION_MODE),
    punctuation_list("punctuation_list","+=<>~@#$%^&*|"),
    capitals_mode("indicate_capitals",RHVoice_capitals_off)
  {
    punctuation_mode.define("none",RHVoice_punctuation_none);
    punctuation_mode.define("some",RHVoice_punctuation_some);
    punctuation_mode.define("all",RHVoice_punctuation_all);
    capitals_mode.define("off",RHVoice_capitals_off);
    capitals_mode.define("no",RHVoice_capitals_off);
    capitals_mode.define("none",RHVoice_capitals_off);
    capitals_mode.define("false",RHVoice_capitals_off);
    capitals_mode.define("word",RHVoice_capitals_word);
    capitals_mode.define("pitch",RHVoice_capitals_pitch);
    capitals_mode.define("sound",RHVoice_capitals_sound);
  }

  void verbosity_params::default_to(const verbosity_params& other)
  {
    punctuation_mode.default_to(other.punctuation_mode);
    punctuation_list.default_to(other.punctuation_list);
    capitals_mode.default_to(other.capitals_mode);
  }

  void verbosity_params::register_self(config& cfg,const std::string& prefix)
  {
    cfg.register_setting(punctuation_mode,prefix);
    cfg.register_setting(punctuation_list,prefix);
    cfg.register_setting(capitals_mode,prefix);
  }

  void stream_params::register_self(config& cfg)
  {
    cfg.register_setting(fixed_size);
    cfg.register_setting(view_size);
  }
}
