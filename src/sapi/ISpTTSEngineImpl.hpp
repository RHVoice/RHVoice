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
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef RHVOICE_SAPI_ISPTTSENGINEIMPL_HPP
#define RHVOICE_SAPI_ISPTTSENGINEIMPL_HPP

#include <windows.h>
#include <sapi.h>
#include <sapiddk.h>
#include <comdef.h>
#include <comip.h>
#include <initguid.h>

#include "core/voice_profile.hpp"
#include "com.hpp"
#include "tts_base.hpp"

namespace RHVoice
{
  namespace sapi
  {
    extern GUID CLSID_ISpTTSEngineImpl;
    class ISpTTSEngineImpl:
      public ISpTTSEngine, public ISpObjectWithToken, public tts_base
    {
    public:
      STDMETHOD(Speak)(DWORD dwSpeakFlags,REFGUID rguidFormatId,const WAVEFORMATEX *pWaveFormatEx,const SPVTEXTFRAG *pTextFragList,ISpTTSEngineSite *pOutputSite);
      STDMETHOD(GetOutputFormat)(const GUID *pTargetFmtId,const WAVEFORMATEX *pTargetWaveFormatEx,GUID *pOutputFormatId,WAVEFORMATEX **ppCoMemOutputWaveFormatEx);
      STDMETHOD(SetObjectToken)(ISpObjectToken *pToken);
      STDMETHOD(GetObjectToken)(ISpObjectToken **ppToken);

    protected:
      void* get_interface(REFIID riid);

    private:
      typedef _com_ptr_t <_com_IIID<ISpObjectToken, &IID_ISpObjectToken>> ISpObjectTokenPtr;
      typedef _com_ptr_t <_com_IIID<ISpDataKey, &IID_ISpDataKey>> ISpDataKeyPtr;


      ISpObjectTokenPtr token;
      voice_profile profile;
    };
  }
}
#endif
