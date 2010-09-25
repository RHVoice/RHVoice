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

#include "SpTTSEngineImpl.h"
#include "global.h"
#include "win32_exception.h"

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
  return S_OK;
}


STDMETHODIMP CSpTTSEngineImpl::GetOutputFormat(const GUID *pTargetFmtId,const WAVEFORMATEX *pTargetWaveFormatEx,GUID *pOutputFormatId,WAVEFORMATEX **ppCoMemOutputWaveFormatEx)
{
  return S_OK;
}


STDMETHODIMP CSpTTSEngineImpl::SetObjectToken(ISpObjectToken *pToken)
{
  object_token.Attach(pToken,true);
  return S_OK;
}

STDMETHODIMP CSpTTSEngineImpl::GetObjectToken(ISpObjectToken **ppToken)
{
  *ppToken=NULL;
  if(!object_token)
    return E_FAIL;
  *ppToken=object_token.GetInterfacePtr();
  reinterpret_cast<IUnknown*>(*ppToken)->AddRef();
  return S_OK;
}

