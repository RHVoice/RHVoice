/* Copyright (C) 2012, 2013  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_SAPI_VOICE_ATTRIBUTES_HPP
#define RHVOICE_SAPI_VOICE_ATTRIBUTES_HPP

#include <string>
#include <sstream>
#include <locale>

#include "core/voice_profile.hpp"
#include "utils.hpp"

namespace RHVoice
{
  namespace sapi
  {
    class voice_attributes
    {
    public:
      explicit voice_attributes(const voice_profile& profile_=voice_profile()):
        profile(profile_)
      {
      }

      std::wstring get_name() const
      {
        return utils::string_to_wstring(profile.get_name());
      }

      std::wstring get_age() const
      {
        return L"Adult";
      }

      std::wstring get_gender() const
      {
        return ((profile.primary()->get_gender()==RHVoice_voice_gender_female)?L"Female":L"Male");
      }

      std::wstring get_language() const
      {
        std::wostringstream s;
        s.imbue(std::locale::classic());
        s << std::hex << std::noshowbase << profile.primary()->get_language()->get_id();
        return s.str();
      }

    private:
      voice_profile profile;
    };
  }
}
#endif
