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
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef RHVOICE_SAPI_REGISTRY_HPP
#define RHVOICE_SAPI_REGISTRY_HPP

#include <windows.h>
#include <string>

#include "core/exception.hpp"

namespace RHVoice
{
  namespace registry
  {
    class error: public exception
    {
    public:
      explicit error(const std::string& msg);
    };

    class key
    {
    public:
      key(HKEY parent,const std::wstring& name,REGSAM access_mask=KEY_READ,bool create=false);

      ~key();

      operator HKEY () const;

      void delete_subkey(const std::wstring& name);

      std::wstring get(const std::wstring& name) const;

      std::wstring get() const;

      void set(const std::wstring& name,const std::wstring& value);

      void set(const std::wstring& value);

    private:
      key(const key&);
      key& operator=(const key&);

      HKEY handle;
    };
  }
}
#endif
