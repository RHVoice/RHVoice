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

#ifndef RHVOICE_SAPI_VOICE_ATTRIBUTES_HPP
#define RHVOICE_SAPI_VOICE_ATTRIBUTES_HPP

#include <string>
#include <sstream>
#include <locale>
#include "core/voice.hpp"
#include "utils.hpp"

namespace RHVoice
{
  namespace sapi
  {
    class voice_attributes
    {
    public:
      explicit voice_attributes(voice_list::const_iterator main,voice_list::const_iterator extra=voice_list::const_iterator()):
        main_voice(main),
        extra_voice(extra)
      {
      }

      bool is_monolingual() const
      {
        return (extra_voice==voice_list::const_iterator());
      }

      std::wstring get_main_speaker() const
      {
        return utils::string_to_wstring(main_voice->get_name());
      }

      std::wstring get_extra_speaker() const
      {
        return (is_monolingual()?std::wstring():utils::string_to_wstring(extra_voice->get_name()));
      }

      std::wstring get_name() const
      {
        std::wstring name(get_main_speaker());
        if(!is_monolingual())
          {
            name+='+';
            name+=get_extra_speaker();
          }
        return name;
      }

      std::wstring get_age() const
      {
        return L"Adult";
      }

      std::wstring get_gender() const
      {
        return ((main_voice->get_gender()==RHVoice_voice_gender_female)?L"Female":L"Male");
      }

      std::wstring get_language() const
      {
        std::wostringstream s;
        s.imbue(std::locale::classic());
        s << std::hex << std::noshowbase << main_voice->get_language()->get_id();
        return s.str();
      }

    private:
      voice_list::const_iterator main_voice,extra_voice;
    };
  }
}
#endif
