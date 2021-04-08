/* Copyright (C) 2012  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_SAPI_DBG_HPP
#define RHVOICE_SAPI_DBG_HPP

#include <windows.h>

#include "utils.hpp"

namespace RHVoice
{
  namespace sapi
  {
    inline void dbg_out(const std::wstring& s)
    {
      std::wstring msg(L"RHVoiceSvr: ");
      msg+=s;
      OutputDebugString(msg.c_str());
    }

    inline void dbg_out(const std::string& s)
    {
      dbg_out(utils::string_to_wstring(s));
    }
  }
}
#endif
