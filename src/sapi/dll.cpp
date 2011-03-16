/* Copyright (C) 2010, 2011  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include <string>
#include <stdexcept>
#include <new>
#include <eh.h>
#include "SpTTSEngineImpl.h"
#include "ClassFactoryImpl.h"
#include "global.h"
#include "win32_exception.h"

using std::wstring;
using std::runtime_error;
using std::bad_alloc;

ULONG svr_ref_count=0;

static const wstring str_svr_clsid=L"{9F215C97-3D3B-489D-8419-6B9ABBF31EC2}";
static const wstring str_svr_key=L"CLSID\\"+str_svr_clsid;
static const wstring str_svr_subkey=str_svr_key+L"\\InProcServer32";
static const wstring str_svr_desc=L"RHVoice";
static const wstring str_svr_threading_model=L"Both";

static HINSTANCE dll_handle=NULL;

static void svrKey_set_string_value(const wstring& key_name,const wstring& value_name,const wstring& data)
{
  HKEY hKey=NULL;
  LONG result;
  result=RegCreateKeyEx(HKEY_CLASSES_ROOT,key_name.c_str(),
                        0,NULL,REG_OPTION_NON_VOLATILE,
                        KEY_SET_VALUE,NULL,&hKey,NULL);
  if(result!=ERROR_SUCCESS)
    throw runtime_error("Unable to create/open a registry key");
  result=RegSetValueEx(hKey,value_name.c_str(),0,REG_SZ,
                       reinterpret_cast<const BYTE*>(data.c_str()),(data.length()+1)*sizeof(wchar_t));
  RegCloseKey(hKey);
  if(result!=ERROR_SUCCESS)
    throw runtime_error("Unable to set a registry value");
}

static void exception_handler(unsigned int code,EXCEPTION_POINTERS *e_ptrs)
{
  throw win32_exception(code);
}

BOOL APIENTRY DllMain(HINSTANCE hInstance,DWORD dwReason,LPVOID lpReserved)
{
  switch(dwReason)
    {
    case DLL_PROCESS_ATTACH:
      dll_handle=hInstance;
      InitializeCriticalSection(&init_mutex);
      break;
    case DLL_THREAD_ATTACH:
      _set_se_translator(exception_handler);
      break;
    default:
      break;
    }
  return TRUE;
}

STDAPI DllGetClassObject(REFCLSID rclsid,REFIID riid,void** ppv)
{
  HRESULT result=S_OK;
  try
    {
      *ppv=NULL;
      if(!IsEqualCLSID(rclsid,__uuidof(CSpTTSEngineImpl)))
        return CLASS_E_CLASSNOTAVAILABLE;
      CClassFactoryImpl *pFactory=new CClassFactoryImpl;
      pFactory->AddRef();
      result=pFactory->QueryInterface(riid,ppv);
      pFactory->Release();
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

STDAPI DllCanUnloadNow()
{
  return (svr_ref_count>0)?S_FALSE:S_OK;
}

STDAPI DllRegisterServer()
{
  HRESULT result=S_OK;
  try
    {
      wchar_t dll_path[MAX_PATH+1];
      DWORD dll_path_length=GetModuleFileName(dll_handle,dll_path,MAX_PATH);
      if(dll_path_length==0)
        return SELFREG_E_CLASS;
      dll_path[dll_path_length]=L'\0';
      svrKey_set_string_value(str_svr_key,L"",str_svr_desc);
      svrKey_set_string_value(str_svr_subkey,L"",dll_path);
      svrKey_set_string_value(str_svr_subkey,L"ThreadingModel",str_svr_threading_model);
    }
  catch(...)
    {
      result=SELFREG_E_CLASS;
    }
  return result;
}

STDAPI DllUnregisterServer()
{
  RegDeleteKey(HKEY_CLASSES_ROOT,str_svr_subkey.c_str());
  RegDeleteKey(HKEY_CLASSES_ROOT,str_svr_key.c_str());
  return S_OK;
}
