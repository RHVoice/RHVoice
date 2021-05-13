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

#include <iterator>
#include "ISpDataKeyImpl.hpp"

namespace RHVoice
{
  namespace sapi
  {
    STDMETHODIMP ISpDataKeyImpl::SetData(LPCWSTR pszValueName,ULONG cbData,const BYTE *pData)
    {
      return E_NOTIMPL;
    }

        STDMETHODIMP ISpDataKeyImpl::GetData(LPCWSTR pszValueName,ULONG *pcbData,BYTE *pData)
    {
      return SPERR_NOT_FOUND;
    }

        STDMETHODIMP ISpDataKeyImpl::SetStringValue(LPCWSTR pszValueName,LPCWSTR pszValue)
    {
      return E_NOTIMPL;
    }

        STDMETHODIMP ISpDataKeyImpl::GetStringValue(LPCWSTR pszValueName,LPWSTR *ppszValue)
    {
      if(ppszValue==0)
        return E_POINTER;
      *ppszValue=0;
      try
        {
          if(pszValueName==0)
            *ppszValue=com::strdup(default_value);
          else
            {
              value_map::const_iterator it=values.find(pszValueName);
              if(it==values.end())
                return SPERR_NOT_FOUND;
              else
                *ppszValue=com::strdup(it->second);
            }
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

        STDMETHODIMP ISpDataKeyImpl::SetDWORD(LPCWSTR pszValueName,DWORD dwValue)
    {
      return E_NOTIMPL;
    }

        STDMETHODIMP ISpDataKeyImpl::GetDWORD(LPCWSTR pszKeyName,DWORD *pdwValue)
    {
      return E_NOTIMPL;
    }

        STDMETHODIMP ISpDataKeyImpl::OpenKey(LPCWSTR pszSubKeyName,ISpDataKey **ppSubKey)
    {
      return SPERR_NOT_FOUND;
    }

        STDMETHODIMP ISpDataKeyImpl::CreateKey(LPCWSTR pszSubKeyName,ISpDataKey **ppSubKey)
    {
      return E_NOTIMPL;
    }

        STDMETHODIMP ISpDataKeyImpl::DeleteKey(LPCWSTR pszSubKeyName)
    {
      return E_NOTIMPL;
    }

        STDMETHODIMP ISpDataKeyImpl::DeleteValue(LPCWSTR pszValueName)
    {
      return E_NOTIMPL;
    }

        STDMETHODIMP ISpDataKeyImpl::EnumKeys(ULONG Index,LPWSTR *ppszSubKeyName)
    {
      return SPERR_NO_MORE_ITEMS;
    }

        STDMETHODIMP ISpDataKeyImpl::EnumValues(ULONG Index,LPWSTR *ppszValueName)
    {
      if(Index>=values.size())
        return SPERR_NO_MORE_ITEMS;
      if(ppszValueName==0)
        return E_POINTER;
      *ppszValueName=0;
      try
        {
          value_map::const_iterator it(values.begin());
          std::advance(it,Index);
          *ppszValueName=com::strdup(it->second);
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
