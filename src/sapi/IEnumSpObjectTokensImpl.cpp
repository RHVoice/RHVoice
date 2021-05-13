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

#include <stdexcept>
#include <new>
#include <algorithm>

#include "IEnumSpObjectTokensImpl.hpp"
#include "core/voice.hpp"
#include "core/language.hpp"

namespace RHVoice
{
  namespace sapi
  {
    IEnumSpObjectTokensImpl::IEnumSpObjectTokensImpl(bool initialize):
      id(SPCAT_VOICES),
      index(0)
    {
      id+=L"\\TokenEnums\\RHVoice";
      if(!initialize)
        return;
      const std::set<voice_profile>& profiles=get_engine()->get_voice_profiles();
      for(std::set<voice_profile>::const_iterator it=profiles.begin();it!=profiles.end();++it)
        {
          sapi_voices.push_back(voice_attributes(*it));
        }
    }

    IEnumSpObjectTokensImpl::ISpObjectTokenPtr IEnumSpObjectTokensImpl::create_token(const voice_attributes& attr) const
    {
      std::wstring token_id=id+std::wstring(L"\\")+attr.get_name();
      com::object<voice_token> obj_data_key(attr);
      com::interface_ptr<ISpDataKey> int_data_key(obj_data_key);
      ISpObjectTokenInitPtr int_token_init(CLSID_SpObjectToken);
      if(!int_token_init)
        throw std::runtime_error("Unable to create an object token");
      if(FAILED(int_token_init->InitFromDataKey(SPCAT_VOICES,token_id.c_str(),int_data_key.get(false))))
        throw std::runtime_error("Unable to initialize an object token");
      ISpObjectTokenPtr int_token=int_token_init;
      return int_token;
    }

    STDMETHODIMP IEnumSpObjectTokensImpl::Next(ULONG celt,ISpObjectToken **pelt,ULONG *pceltFetched)
    {
      if(celt==0)
        return E_INVALIDARG;
      if(pelt==0)
        return E_POINTER;
      if((pceltFetched==0)&&(celt>1))
        return E_POINTER;
      if(pceltFetched!=0)
        *pceltFetched=0;
      try
        {
          std::vector<ISpObjectTokenPtr> tokens;
          unsigned long next_index=std::min(index+celt,static_cast<unsigned long>(sapi_voices.size()));
          for(unsigned long i=index;i<next_index;++i)
            {
              tokens.push_back(create_token(sapi_voices[i]));
            }
          for(unsigned long i=0;i<tokens.size();++i)
            {
              tokens[i].AddRef();
              pelt[i]=tokens[i].GetInterfacePtr();
            }
          if(pceltFetched)
            *pceltFetched=tokens.size();
          index+=tokens.size();
          return ((tokens.size()==celt)?S_OK:S_FALSE);
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

    STDMETHODIMP IEnumSpObjectTokensImpl::Skip(ULONG celt)
    {
      unsigned long num_skipped=std::min<unsigned long>(celt,sapi_voices.size()-index);
      index+=num_skipped;
      return ((num_skipped==celt)?S_OK:S_FALSE);
    }

    STDMETHODIMP IEnumSpObjectTokensImpl::Reset()
    {
      index=0;
      return S_OK;
    }

    STDMETHODIMP IEnumSpObjectTokensImpl::GetCount(ULONG *pulCount)
    {
      if(pulCount==0)
        return E_POINTER;
      *pulCount=sapi_voices.size();
      return S_OK;
    }

    STDMETHODIMP IEnumSpObjectTokensImpl::Item(ULONG Index,ISpObjectToken **ppToken)
    {
      if(ppToken==0)
        return E_POINTER;
      *ppToken=0;
      if(Index>=sapi_voices.size())
        return SPERR_NO_MORE_ITEMS;
      try
        {
          ISpObjectTokenPtr int_token=create_token(sapi_voices[Index]);
          int_token.AddRef();
          *ppToken=int_token.GetInterfacePtr();
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

    STDMETHODIMP IEnumSpObjectTokensImpl::Clone(IEnumSpObjectTokens **ppEnum)
    {
      if(ppEnum==0)
        return E_POINTER;
      *ppEnum=0;
      try
        {
          com::object<IEnumSpObjectTokensImpl> obj(false);
          obj->sapi_voices=sapi_voices;
          obj->index=index;
          com::interface_ptr<IEnumSpObjectTokens> int_ptr(obj);
          *ppEnum=int_ptr.get();
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
