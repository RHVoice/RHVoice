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
#include "ClassFactoryImpl.h"
#include "SpTTSEngineImpl.h"
#include "global.h"
#include "win32_exception.h"

using std::bad_alloc;

CClassFactoryImpl::CClassFactoryImpl()
{
  ref_count=0;
  InterlockedIncrement(&svr_ref_count);
}

CClassFactoryImpl::~CClassFactoryImpl()
{
  InterlockedDecrement(&svr_ref_count);
}

STDMETHODIMP_(ULONG) CClassFactoryImpl::AddRef()
{
  return ++ref_count;
}

STDMETHODIMP_(ULONG) CClassFactoryImpl::Release()
{
  if(--ref_count==0)
    {
      delete this;
      return 0;
    }
  return ref_count;
}

STDMETHODIMP CClassFactoryImpl::QueryInterface(REFIID riid,void** ppv)
{
  HRESULT result=S_OK;
  try
    {
      *ppv=NULL;
      if(IsEqualIID(riid,IID_IUnknown))
        *ppv=static_cast<IUnknown*>(this);
      else if(IsEqualIID(riid,IID_IClassFactory))
        *ppv=static_cast<IClassFactory*>(this);
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

STDMETHODIMP CClassFactoryImpl::CreateInstance(IUnknown* pUnkOuter,REFIID riid,void** ppv)
{
  HRESULT result=S_OK;
  try
    {
  *ppv=NULL;
  if(pUnkOuter!=NULL)
    return CLASS_E_NOAGGREGATION;
  CSpTTSEngineImpl *pObj=new CSpTTSEngineImpl;
  pObj->AddRef();
  result=pObj->QueryInterface(riid,ppv);
  pObj->Release();
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

STDMETHODIMP CClassFactoryImpl::LockServer(BOOL fLock)
{
  if(fLock)
    InterlockedIncrement(&svr_ref_count);
  else
    InterlockedDecrement(&svr_ref_count);
  return S_OK;
}
