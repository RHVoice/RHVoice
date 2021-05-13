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
#include "com.hpp"
#include "ISpTTSEngineImpl.hpp"
#include "IEnumSpObjectTokensImpl.hpp"

namespace
{
  HINSTANCE dll_handle=0;
  RHVoice::com::class_object_factory cls_obj_factory;
}

BOOL APIENTRY DllMain(HINSTANCE hInstance,DWORD dwReason,  [[maybe_unused]] LPVOID lpReserved)
{
  if(dwReason==DLL_PROCESS_ATTACH)
    {
      dll_handle=hInstance;
      try
        {
          cls_obj_factory.register_class<RHVoice::sapi::IEnumSpObjectTokensImpl>();
          cls_obj_factory.register_class<RHVoice::sapi::ISpTTSEngineImpl>();
        }
      catch(...)
        {
          return FALSE;
        }
    }
  return TRUE;
}

STDAPI DllGetClassObject(REFCLSID rclsid,REFIID riid,void** ppv)
{
  try
    {
      return cls_obj_factory.create(rclsid,riid,ppv);
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

STDAPI DllCanUnloadNow()
{
  return RHVoice::com::object_counter::is_zero();
}

STDAPI DllRegisterServer()
{
  try
    {
      RHVoice::com::class_registrar r(dll_handle);
      r.register_class<RHVoice::sapi::IEnumSpObjectTokensImpl>();
      r.register_class<RHVoice::sapi::ISpTTSEngineImpl>();
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

STDAPI DllUnregisterServer()
{
  try
    {
      RHVoice::com::class_registrar r(dll_handle);
      r.unregister_class<RHVoice::sapi::IEnumSpObjectTokensImpl>();
      r.unregister_class<RHVoice::sapi::ISpTTSEngineImpl>();
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
