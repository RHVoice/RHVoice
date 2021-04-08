/* Copyright (C) 2012  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 2 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#ifndef RHVOICE_SD_TTS_PARAMS_HPP
#define RHVOICE_SD_TTS_PARAMS_HPP

#include "RHVoice_common.h"
#include "core/config.hpp"

namespace RHVoice
{
  namespace sd
  {
    enum voice_id_t
      {
        voice_male3=-3,
        voice_male2=-2,
        voice_male1=-1,
        voice_default=0,
        voice_female1=1,
        voice_female2=2,
        voice_female3=3
      };

    class tts_params
    {
    public:
      numeric_property<int> rate;
      numeric_property<int> pitch;
      numeric_property<int> volume;
      c_enum_property<RHVoice_punctuation_mode> punctuation_mode;
      bool_property spelling_mode;
      c_enum_property<RHVoice_capitals_mode> capitals_mode;
      enum_string_property language_code;
      enum_property<voice_id_t> voice_id;
      enum_string_property voice_name;

      tts_params():
        rate("rate",0,-100,100),
        pitch("pitch",0,-100,100),
        volume("volume",0,-100,100),
        punctuation_mode("punctuation_mode",RHVoice_punctuation_default),
        spelling_mode("spelling_mode",false),
        capitals_mode("cap_let_recogn",RHVoice_capitals_default),
        language_code("language"),
        voice_id("voice",voice_default),
        voice_name("synthesis_voice")
      {
        punctuation_mode.define("none",RHVoice_punctuation_none);
        punctuation_mode.define("some",RHVoice_punctuation_some);
        punctuation_mode.define("all",RHVoice_punctuation_all);
        capitals_mode.define("none",RHVoice_capitals_off);
        capitals_mode.define("spell",RHVoice_capitals_word);
        capitals_mode.define("icon",RHVoice_capitals_sound);
        voice_id.define("male1",voice_male1);
        voice_id.define("male2",voice_male2);
        voice_id.define("male3",voice_male3);
        voice_id.define("female1",voice_female1);
        voice_id.define("female2",voice_female2);
        voice_id.define("female3",voice_female3);
      }

      void register_self(config& conf)
      {
        conf.register_setting(rate);
        conf.register_setting(pitch);
        conf.register_setting(volume);
        conf.register_setting(punctuation_mode);
        conf.register_setting(punctuation_mode);
        conf.register_setting(spelling_mode);
        conf.register_setting(capitals_mode);
        conf.register_setting(language_code);
        conf.register_setting(voice_id);
        conf.register_setting(voice_name);
      }

    private:
      tts_params(const tts_params&);
      tts_params& operator=(const tts_params&);
    };
  }
}
#endif
