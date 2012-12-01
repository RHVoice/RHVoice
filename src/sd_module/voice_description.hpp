/* Copyright (C) 2012  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_SD_VOICE_DESCRIPTION_HPP
#define RHVOICE_SD_VOICE_DESCRIPTION_HPP

#include "core/language.hpp"
#include "core/voice.hpp"

namespace RHVoice
{
  namespace sd
  {
    class voice_description
    {
    public:
      explicit voice_description(voice_list::const_iterator voice1=voice_list::const_iterator(),voice_list::const_iterator voice2=voice_list::const_iterator()):
        main_voice(voice1),
        extra_voice(voice2)
      {
      }

      voice_list::const_iterator get_main_voice() const
      {
        return main_voice;
      }

      voice_list::const_iterator get_extra_voice() const
      {
        return extra_voice;
      }

      std::string get_name() const
      {
        std::string name;
        if(main_voice!=voice_list::const_iterator())
          {
            name=main_voice->get_name();
            if(extra_voice!=voice_list::const_iterator())
              {
                name+="+";
                name+=extra_voice->get_name();
                }
          }
        return name;
      }

      std::string get_language_code() const
      {
        std::string code;
        if(main_voice!=voice_list::const_iterator())
          code=main_voice->get_language()->get_alpha2_code();
        return code;
      }

      bool is_monolingual() const
      {
        return ((main_voice!=voice_list::const_iterator())&&(extra_voice==voice_list::const_iterator()));
      }

      bool is_bilingual() const
      {
        return ((main_voice!=voice_list::const_iterator())&&(extra_voice!=voice_list::const_iterator()));
      }

      bool is_empty() const
      {
        return ((main_voice==voice_list::const_iterator())&&(extra_voice==voice_list::const_iterator()));
      }

    private:
      voice_list::const_iterator main_voice,extra_voice;
    };
  }
}
#endif
