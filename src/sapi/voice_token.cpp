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

#include <new>
#include <cstring>
#include <comdef.h>
#include "voice_token.hpp"
#include "ISpTTSEngineImpl.hpp"

namespace RHVoice
{
  namespace sapi
  {
    voice_token::voice_token(const voice_attributes& attr)
    {
      std::wstring name(attr.get_name());
      set(name);
      set(L"CLSID",com::clsid_as_string<ISpTTSEngineImpl>());
      attributes[L"Age"]=attr.get_age();
      attributes[L"Vendor"]=L"Olga Yakovleva";
      attributes[L"Language"]=attr.get_language();
      attributes[L"Gender"]=attr.get_gender();
      attributes[L"Name"]=name;
    }

    [[gnu::nothrow]] STDMETHODIMP voice_token::OpenKey(LPCWSTR pszSubKeyName,ISpDataKey **ppSubKey)
    {
      if(pszSubKeyName==0)
        return E_INVALIDARG;
      if(ppSubKey==0)
        return E_POINTER;
      *ppSubKey=0;
      try
        {
          if(!str_equal(pszSubKeyName,L"Attributes"))
            return SPERR_NOT_FOUND;
          com::object<ISpDataKeyImpl > obj;
          for(attribute_map::const_iterator it=attributes.begin();it!=attributes.end();++it)
            {
              obj->set(it->first,it->second);
            }
          com::interface_ptr<ISpDataKey> int_ptr(obj);
          *ppSubKey=int_ptr.get();
          return S_OK;
        }
      catch(const std::bad_alloc&)
        {
          return E_OUTOFMEMORY;
        }
      catch(...)
        {
          return E_UNEXPECTED;
        }
    }

    [[gnu::nothrow]] STDMETHODIMP voice_token::EnumKeys(ULONG Index,LPWSTR *ppszSubKeyName)
    {
      if(ppszSubKeyName==0)
        return E_POINTER;
      *ppszSubKeyName=0;
      if(Index>0)
        return SPERR_NO_MORE_ITEMS;
      try
        {
          *ppszSubKeyName=com::strdup(L"Attributes");
          return S_OK;
        }
      catch(const std::bad_alloc&)
        {
          return E_OUTOFMEMORY;
        }
      catch(...)
        {
          return E_UNEXPECTED;
        }
    }
  }
}
