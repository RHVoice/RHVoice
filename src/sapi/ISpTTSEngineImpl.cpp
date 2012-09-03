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
          const voice_list& voices=get_engine()->get_voices();
          voice_list::const_iterator v;
          utils::out_ptr<wchar_t> name(CoTaskMemFree);
          if(FAILED(attr->GetStringValue(L"MainSpeaker",name.address())))
            return E_INVALIDARG;
          v=voices.find(utils::wstring_to_string(name.get()));
          if(v==voices.end())
            return E_INVALIDARG;
          main_voice=v;
          extra_voice=voice_list::const_iterator();
          if(SUCCEEDED(attr->GetStringValue(L"ExtraSpeaker",name.address())))
            {
              v=voices.find(utils::wstring_to_string(name.get()));
              if(v!=voices.end())
                extra_voice=v;
            }
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

    STDMETHODIMP ISpTTSEngineImpl::GetOutputFormat(const GUID *pTargetFmtId,const WAVEFORMATEX *pTargetWaveFormatEx,GUID *pOutputFormatId,WAVEFORMATEX **ppCoMemOutputWaveFormatEx)
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
      pwfex->nSamplesPerSec=16000;
      pwfex->wBitsPerSample=16;
      pwfex->nBlockAlign=pwfex->nChannels*pwfex->wBitsPerSample/8;
      pwfex->nAvgBytesPerSec=pwfex->nSamplesPerSec*pwfex->nBlockAlign;
      pwfex->cbSize=0;
      *ppCoMemOutputWaveFormatEx=pwfex;
      return S_OK;
    }

    STDMETHODIMP ISpTTSEngineImpl::Speak(DWORD dwSpeakFlags,REFGUID rguidFormatId,const WAVEFORMATEX *pWaveFormatEx,const SPVTEXTFRAG *pTextFragList,ISpTTSEngineSite *pOutputSite)
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
          p.document_params.main_voice=main_voice;
          p.document_params.extra_voice=extra_voice;
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
