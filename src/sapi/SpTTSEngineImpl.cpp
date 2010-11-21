/* Copyright (C) 2010  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <new>
#include "SpTTSEngineImpl.h"
#include "global.h"
#include "win32_exception.h"
#include "lib.h"
#include "SpeakImpl.h"
#include "util.h"

using std::string;
using boost::shared_ptr;
using std::bad_alloc;

CSpTTSEngineImpl::CSpTTSEngineImpl()
{
  ref_count=0;
  InterlockedIncrement(&svr_ref_count);
}

CSpTTSEngineImpl::~CSpTTSEngineImpl()
{
  InterlockedDecrement(&svr_ref_count);
}

STDMETHODIMP_(ULONG) CSpTTSEngineImpl::AddRef()
{
  return ++ref_count;
}

STDMETHODIMP_(ULONG) CSpTTSEngineImpl::Release()
{
  if(--ref_count==0)
    {
      delete this;
      return 0;
    }
  return ref_count;
}

STDMETHODIMP CSpTTSEngineImpl::QueryInterface(REFIID riid,void** ppv)
{
  HRESULT result=S_OK;
  try
    {
      *ppv=NULL;
      if(IsEqualIID(riid,IID_IUnknown))
        *ppv=static_cast<IUnknown*>(static_cast<ISpTTSEngine*>(this));
      else if(IsEqualIID(riid,IID_ISpTTSEngine))
        *ppv=static_cast<ISpTTSEngine*>(this);
      else if(IsEqualIID(riid,IID_ISpObjectWithToken))
        *ppv=static_cast<ISpObjectWithToken*>(this);
      else
        result=E_NOINTERFACE;
      if(result==S_OK)
        reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    }
  catch(win32_exception& e)
    {
      result=(e.code==EXCEPTION_ACCESS_VIOLATION)?E_POINTER:E_UNEXPECTED;
    }
  return result;
}

STDMETHODIMP CSpTTSEngineImpl::Speak(DWORD dwSpeakFlags,REFGUID rguidFormatId,const WAVEFORMATEX *pWaveFormatEx,const SPVTEXTFRAG *pTextFragList,ISpTTSEngineSite *pOutputSite)
{
  HRESULT result=S_OK;
  try
    {
      if((pTextFragList==NULL)||(pOutputSite==NULL))
        return E_INVALIDARG;
      if(!voice)
        return E_UNEXPECTED;
      SpeakImpl speak(pTextFragList,pOutputSite,voice.get());
      result=speak();
    }
  catch(bad_alloc&)
    {
      result=E_OUTOFMEMORY;
    }
  catch(win32_exception& e)
    {
      result=(e.code==EXCEPTION_ACCESS_VIOLATION)?E_POINTER:E_UNEXPECTED;
    }
  catch(...)
    {
      result=E_UNEXPECTED;
    }
  return result;
}

STDMETHODIMP CSpTTSEngineImpl::GetOutputFormat(const GUID *pTargetFmtId,const WAVEFORMATEX *pTargetWaveFormatEx,GUID *pOutputFormatId,WAVEFORMATEX **ppCoMemOutputWaveFormatEx)
{
  HRESULT result=S_OK;
  try
    {
      if((pOutputFormatId==NULL)||(ppCoMemOutputWaveFormatEx==NULL))
        return E_INVALIDARG;
      if(!voice)
        return E_UNEXPECTED;
      int sample_rate=get_param_int(voice->features,"sample_rate",16000);
      *pOutputFormatId=SPDFID_WaveFormatEx;
      *ppCoMemOutputWaveFormatEx=NULL;
      WAVEFORMATEX *pwfex=reinterpret_cast<WAVEFORMATEX*>(CoTaskMemAlloc(sizeof(WAVEFORMATEX)));
      if(pwfex==NULL)
        return E_OUTOFMEMORY;
      pwfex->wFormatTag=0x0001;
      pwfex->nChannels=1;
      pwfex->nSamplesPerSec=sample_rate;
      pwfex->wBitsPerSample=16;
      pwfex->nBlockAlign=pwfex->nChannels*pwfex->wBitsPerSample/8;
      pwfex->nAvgBytesPerSec=pwfex->nSamplesPerSec*pwfex->nBlockAlign;
      pwfex->cbSize=0;
      *ppCoMemOutputWaveFormatEx=pwfex;
    }
  catch(win32_exception& e)
    {
      result=(e.code==EXCEPTION_ACCESS_VIOLATION)?E_POINTER:E_UNEXPECTED;
    }
  catch(...)
    {
      result=E_UNEXPECTED;
    }
  return result;
}

STDMETHODIMP CSpTTSEngineImpl::SetObjectToken(ISpObjectToken *pToken)
{
  HRESULT result=S_OK;
  try
    {
      if(pToken==NULL)
        return E_INVALIDARG;
      wchar_t *voice_path_w=NULL;
      if(FAILED(pToken->GetStringValue(L"VoicePath",&voice_path_w)))
        return E_FAIL;
      shared_ptr<wchar_t> guard(voice_path_w,CoTaskMemFree);
      string voice_path=wstring_to_string(voice_path_w,false);
      shared_ptr<cst_voice> voice0(RHVoice_create_voice(voice_path.c_str(),SpeakImpl::callback),RHVoice_delete_voice);
      if(!voice0)
        return E_FAIL;
      voice=voice0;
      object_token.Attach(pToken,true);
    }
  catch(bad_alloc&)
    {
      result=E_OUTOFMEMORY;
    }
  catch(win32_exception& e)
    {
      result=(e.code==EXCEPTION_ACCESS_VIOLATION)?E_POINTER:E_UNEXPECTED;
    }
  catch(...)
    {
      result=E_UNEXPECTED;
    }
  return result;
}

STDMETHODIMP CSpTTSEngineImpl::GetObjectToken(ISpObjectToken **ppToken)
{
  HRESULT result=S_OK;
  try
    {
      *ppToken=NULL;
      if(!object_token)
        return E_FAIL;
      *ppToken=object_token.GetInterfacePtr();
      (*ppToken)->AddRef();
    }
  catch(win32_exception& e)
    {
      result=(e.code==EXCEPTION_ACCESS_VIOLATION)?E_POINTER:E_UNEXPECTED;
    }
  catch(...)
    {
      result=E_UNEXPECTED;
    }
  return result;
}
