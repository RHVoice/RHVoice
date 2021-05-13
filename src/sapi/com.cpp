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

#include <algorithm>
#include <stdexcept>
#include "com.hpp"

namespace RHVoice
{
  namespace com
  {
    wchar_t* strdup(const std::wstring& s)
    {
      std::size_t size=s.size();
      wchar_t* b=reinterpret_cast<wchar_t*>(CoTaskMemAlloc((size+1)*sizeof(wchar_t)));
      if(b==0)
        throw std::bad_alloc();
      std::copy(s.begin(),s.end(),b);
      b[size]=L'\0';
      return b;
    }

    volatile long object_counter::count=0;

    HRESULT class_object_factory::create(REFCLSID rclsid,REFIID riid,void** ppv) const
    {
      if(ppv==0)
        return E_POINTER;
      *ppv=0;
      for(std::vector<creator_ptr>::const_iterator it=creators.begin();it!=creators.end();++it)
        {
          creator_ptr cp=*it;
          if(cp->matches(rclsid))
            return cp->create(riid,ppv);
        }
      return CLASS_E_CLASSNOTAVAILABLE;
    }

    class_registrar::class_registrar(HINSTANCE dll_handle)
    {
      wchar_t buffer[MAX_PATH+1];
      DWORD size=GetModuleFileName(dll_handle,buffer,MAX_PATH);
      if(size==0)
        throw std::runtime_error("Unable to get the path of the dll");
      buffer[size]=L'\0';
      dll_path.assign(buffer);
    }

    const std::wstring class_registrar::clsid_key_path(L"Software\\Classes\\CLSID");
  }
}
