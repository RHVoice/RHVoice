/* Copyright (C) 2012  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include "core/params.hpp"

namespace RHVoice
{
  voice_params::voice_params():
    default_rate("default_rate",1,0.2,5),
    min_rate("min_rate",0.5,0.2,1),
    max_rate("max_rate",2,1,5),
    default_pitch("default_pitch",1,0.5,2),
    min_pitch("min_pitch",0.5,0.5,1),
    max_pitch("max_pitch",2,1,2),
    default_volume("default_volume",1,0.25,2),
    min_volume("min_volume",0.25,0.25,1),
    max_volume("max_volume",2,1,2)
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
}
