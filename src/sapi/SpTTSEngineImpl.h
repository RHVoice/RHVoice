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

#ifndef SPTTSENGINEIMPL_H
#define SPTTSENGINEIMPL_H

#include <windows.h>
#include <comdef.h>
#include <sapi.h>
#include <sapiddk.h>

class __declspec(uuid("{9f215c97-3d3b-489d-8419-6b9abbf31ec2}")) CSpTTSEngineImpl: public ISpTTSEngine,public ISpObjectWithToken
{
 public:
  CSpTTSEngineImpl();
  virtual ~CSpTTSEngineImpl();
  STDMETHOD_(ULONG,AddRef)();
  STDMETHOD_(ULONG,Release)();
  STDMETHOD(QueryInterface)(REFIID riid,void** ppv);
  STDMETHOD(Speak)(DWORD dwSpeakFlags,REFGUID rguidFormatId,const WAVEFORMATEX *pWaveFormatEx,const SPVTEXTFRAG *pTextFragList,ISpTTSEngineSite *pOutputSite);
  STDMETHOD(GetOutputFormat)(const GUID *pTargetFmtId,const WAVEFORMATEX *pTargetWaveFormatEx,GUID *pOutputFormatId,WAVEFORMATEX **ppCoMemOutputWaveFormatEx);
  STDMETHOD(SetObjectToken)(ISpObjectToken *pToken);
  STDMETHOD(GetObjectToken)(ISpObjectToken **ppToken);
 private:

  _COM_SMARTPTR_TYPEDEF(ISpObjectToken,IID_ISpObjectToken);

  ULONG ref_count;
  ISpObjectTokenPtr object_token;
};

#endif
