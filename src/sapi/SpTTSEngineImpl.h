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

#ifndef SPTTSENGINEIMPL_H
#define SPTTSENGINEIMPL_H

#include <windows.h>
#include <comdef.h>
#include <sapi.h>
#include <sapiddk.h>
#include <string>
#include <sstream>
#include <map>
#include "RHVoice.h"

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
  static int sample_rate;
  std::wstring ssml;
  std::map<size_t,const SPVTEXTFRAG*> frag_map;
  ISpTTSEngineSite *out;
  unsigned long long audio_bytes;
  static float pitch_table[];
  static float rate_table[];

  static void write_text_to_stream(std::wostringstream& s,const wchar_t *text,size_t len);

static   std::wstring::const_iterator skip_unichar(std::wstring::const_iterator it)
    {
      return ((*it<0xd800)||(*it>=0xe000))?(it+1):(it+2);
    }

  std::wstring::const_iterator skip_unichars(std::wstring::const_iterator first,size_t n);
  unsigned long convert_position(std::wstring::const_iterator ssml_pos);
  void generate_ssml(const SPVTEXTFRAG *frags);
  static int callback(const short *samples,int num_samples,const RHVoice_event *events,int num_events,RHVoice_message message);
  int real_callback(const short *samples,int num_samples,const RHVoice_event *events,int num_events);
  static float get_pitch_factor(int value);
  static float get_rate_factor(int value);
  static float convert_volume(unsigned int value);
  void set_rate();
  void set_volume();
  int get_sample_rate();
};

#endif
