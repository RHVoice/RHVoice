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

#ifndef RHVOICE_SAPI_ISPTTSENGINEIMPL_HPP
#define RHVOICE_SAPI_ISPTTSENGINEIMPL_HPP

#include <windows.h>
#include <sapi.h>
#include <sapiddk.h>
#include <comdef.h>
#include <comip.h>
#include "core/voice_profile.hpp"
#include "com.hpp"
#include "tts_base.hpp"

namespace RHVoice
{
  namespace sapi
  {
    class __declspec(uuid("{9f215c97-3d3b-489d-8419-6b9abbf31ec2}")) ISpTTSEngineImpl:
      public ISpTTSEngine, public ISpObjectWithToken, public tts_base
    {
    public:
      STDMETHOD(Speak)(DWORD dwSpeakFlags,REFGUID rguidFormatId,const WAVEFORMATEX *pWaveFormatEx,const SPVTEXTFRAG *pTextFragList,ISpTTSEngineSite *pOutputSite);
      STDMETHOD(GetOutputFormat)(const GUID *pTargetFmtId,const WAVEFORMATEX *pTargetWaveFormatEx,GUID *pOutputFormatId,WAVEFORMATEX **ppCoMemOutputWaveFormatEx);
      STDMETHOD(SetObjectToken)(ISpObjectToken *pToken);
      STDMETHOD(GetObjectToken)(ISpObjectToken **ppToken);

    protected:
      void* get_interface(REFIID riid)
      {
        void* ptr=com::try_primary_interface<ISpTTSEngine>(this,riid);
        return (ptr?ptr:com::try_interface<ISpObjectWithToken>(this,riid));
      }

    private:
      _COM_SMARTPTR_TYPEDEF(ISpObjectToken,__uuidof(ISpObjectToken));
      _COM_SMARTPTR_TYPEDEF(ISpDataKey,__uuidof(ISpDataKey));
      ISpObjectTokenPtr token;
      voice_profile profile;
    };
  }
}
#endif
