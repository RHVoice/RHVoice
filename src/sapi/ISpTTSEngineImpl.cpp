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

#include <new>
#include "utils.hpp"
#include "ISpTTSEngineImpl.hpp"
#include "SpeakImpl.hpp"

namespace RHVoice
{
  namespace sapi
  {
    STDMETHODIMP ISpTTSEngineImpl::SetObjectToken(ISpObjectToken *pToken)
    {
      if(pToken==0)
        return E_INVALIDARG;
      try
        {
          ISpDataKeyPtr attr;
          if(FAILED(pToken->OpenKey(L"Attributes",&attr)))
            return E_INVALIDARG;
          utils::out_ptr<wchar_t> name(CoTaskMemFree);
          if(FAILED(attr->GetStringValue(L"Name",name.address())))
            return E_INVALIDARG;
          voice_profile new_profile=get_engine()->create_voice_profile(utils::wstring_to_string(name.get()));
          if(new_profile.empty())
            return E_INVALIDARG;
          profile=new_profile;
          token=pToken;
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

    STDMETHODIMP ISpTTSEngineImpl::GetObjectToken(ISpObjectToken **ppToken)
    {
      if(ppToken==0)
        return E_POINTER;
      *ppToken=0;
      if(token)
        {
          token.AddRef();
          *ppToken=token.GetInterfacePtr();
          return S_OK;
        }
      else
        return E_UNEXPECTED;
    }

    STDMETHODIMP ISpTTSEngineImpl::GetOutputFormat([[maybe_unused]] const GUID *pTargetFmtId, [[maybe_unused]] const WAVEFORMATEX *pTargetWaveFormatEx,GUID *pOutputFormatId,WAVEFORMATEX **ppCoMemOutputWaveFormatEx)
    {
      if(pOutputFormatId==0)
        return E_POINTER;
      if(ppCoMemOutputWaveFormatEx==0)
        return E_POINTER;
      *pOutputFormatId=SPDFID_WaveFormatEx;
      *ppCoMemOutputWaveFormatEx=0;
      WAVEFORMATEX *pwfex=reinterpret_cast<WAVEFORMATEX*>(CoTaskMemAlloc(sizeof(WAVEFORMATEX)));
      if(pwfex==0)
        return E_OUTOFMEMORY;
      pwfex->wFormatTag=0x0001;
      pwfex->nChannels=1;
      pwfex->nSamplesPerSec=((get_engine()->quality==quality_min)?16000:24000);
      pwfex->wBitsPerSample=16;
      pwfex->nBlockAlign=pwfex->nChannels*pwfex->wBitsPerSample/8;
      pwfex->nAvgBytesPerSec=pwfex->nSamplesPerSec*pwfex->nBlockAlign;
      pwfex->cbSize=0;
      *ppCoMemOutputWaveFormatEx=pwfex;
      return S_OK;
    }

    STDMETHODIMP ISpTTSEngineImpl::Speak([[maybe_unused]] DWORD dwSpeakFlags, [[maybe_unused]] REFGUID rguidFormatId, [[maybe_unused]] const WAVEFORMATEX *pWaveFormatEx,const SPVTEXTFRAG *pTextFragList,ISpTTSEngineSite *pOutputSite)
    {
      if(pTextFragList==0)
        return E_INVALIDARG;
      if(pOutputSite==0)
        return E_INVALIDARG;
      try
        {
          SpeakImpl::init_params p;
          p.input=pTextFragList;
          p.caller=pOutputSite;
          p.profile=profile;
          SpeakImpl s(p);
          s();
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
