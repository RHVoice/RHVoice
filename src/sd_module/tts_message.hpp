/* Copyright (C) 2012, 2013  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_SD_TTS_MESSAGE_HPP
#define RHVOICE_SD_TTS_MESSAGE_HPP

#include "tts_params.hpp"
#include "core/voice_profile.hpp"

namespace RHVoice
{
  namespace sd
  {
    enum tts_message_type
      {
        tts_message_text,
        tts_message_char,
        tts_message_key,
        tts_message_icon
      };

    struct tts_message
    {
      tts_message():
        type(tts_message_text),
        rate(0),
        pitch(0),
        volume(0),
        punctuation_mode(RHVoice_punctuation_default),
        spelling_mode(false),
        capitals_mode(RHVoice_capitals_default)
      {
      }

      void set_synthesis_params(const tts_params& p)
      {
        rate=0.01*p.rate;
        pitch=0.01*p.pitch;
        volume=0.01*p.volume;
        punctuation_mode=p.punctuation_mode;
        spelling_mode=p.spelling_mode;
        capitals_mode=p.capitals_mode;
      }

      tts_message_type type;
      std::string content;
      voice_profile profile;
      double rate,pitch,volume;
      RHVoice_punctuation_mode punctuation_mode;
      bool spelling_mode;
      RHVoice_capitals_mode capitals_mode;
    };
  }
}
#endif
