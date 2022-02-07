/* Copyright (C) 2012, 2018, 2021  Olga Yakovleva <olga@rhvoice.org> */

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

#ifndef RHVOICE_TTS_MARKUP_HPP
#define RHVOICE_TTS_MARKUP_HPP

#include <string>
#include "language.hpp"
#include "voice.hpp"

namespace RHVoice
{
  enum content_type
    {
      content_text,
      content_char,
      content_chars,
      content_glyphs,
      content_key,
      content_emoji,
      content_phones
    };

  struct prosodic_attributes
  {
    double rate,pitch,volume;

    prosodic_attributes():
      rate(1.0),
      pitch(1.0),
      volume(1.0)
    {
    }
  };

  struct tts_markup
  {
    bool autosplit_sentences;
    language_search_criteria language_criteria;
    voice_search_criteria voice_criteria;
    content_type say_as;
    prosodic_attributes prosody;

    tts_markup():
      autosplit_sentences(true),
      say_as(content_text)
    {
    }
  };
}
#endif
