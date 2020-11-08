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
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include "voice_attributes.hpp"


namespace RHVoice
{
  namespace sapi
  {
      voice_attributes::voice_attributes(const voice_profile& profile_):
        profile(profile_)
      {
      }

      std::wstring voice_attributes::get_name() const
      {
        return utils::string_to_wstring(profile.get_name());
      }

      std::wstring voice_attributes::get_age() const
      {
        return L"Adult";
      }

      std::wstring voice_attributes::get_gender() const
      {
        return ((profile.primary()->get_gender()==RHVoice_voice_gender_female)?L"Female":L"Male");
      }

      std::wstring voice_attributes::get_language() const
      {
        std::wostringstream s;
        s.imbue(std::locale::classic());
        s << std::hex << std::noshowbase << profile.primary()->get_language()->get_id();
        return s.str();
      }
  }
}
