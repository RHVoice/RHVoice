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

#include <new>
#include <locale>
#include <algorithm>
#include <stdexcept>
#include <cmath>
#include <shlobj.h>
#include "SpTTSEngineImpl.h"
#include "global.h"
#include "win32_exception.h"
#include "util.h"

using std::bad_alloc;
using std::string;
using std::wstring;
using std::wistringstream;
using std::wostringstream;
using std::locale;
using std::map;
using std::min;
using std::max;
using std::pow;
using std::count;
using std::runtime_error;

CRITICAL_SECTION init_mutex;

int CSpTTSEngineImpl::sample_rate=0;

CSpTTSEngineImpl::CSpTTSEngineImpl()
{
  InitializeCriticalSection(&object_token_mutex);
  ref_count=0;
  InterlockedIncrement(&svr_ref_count);
  variant=1;
}

CSpTTSEngineImpl::~CSpTTSEngineImpl()
{
  DeleteCriticalSection(&object_token_mutex);
  InterlockedDecrement(&svr_ref_count);
}

STDMETHODIMP_(ULONG) CSpTTSEngineImpl::AddRef()
{
  return InterlockedIncrement(&ref_count);
}

STDMETHODIMP_(ULONG) CSpTTSEngineImpl::Release()
{
  ULONG new_count=InterlockedDecrement(&ref_count);
  if(new_count==0)
    delete this;
  return new_count;
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
      if(get_sample_rate()==0)
        return E_UNEXPECTED;
      TTSTask t(pTextFragList,pOutputSite,voice,variant,dwSpeakFlags&SPF_NLP_SPEAK_PUNC);
      t();
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
      if(get_sample_rate()==0)
        return E_UNEXPECTED;
      *pOutputFormatId=SPDFID_WaveFormatEx;
      *ppCoMemOutputWaveFormatEx=NULL;
      WAVEFORMATEX *pwfex=reinterpret_cast<WAVEFORMATEX*>(CoTaskMemAlloc(sizeof(WAVEFORMATEX)));
      if(pwfex==NULL)
        return E_OUTOFMEMORY;
      pwfex->wFormatTag=0x0001;
      pwfex->nChannels=1;
      pwfex->nSamplesPerSec=get_sample_rate();
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
  wchar_t *name=NULL;
  try
    {
      if(pToken==NULL)
        return E_INVALIDARG;
      if(get_sample_rate()==0)
        {
          wstring data_path_w;
          HKEY k;
          if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,L"Software\\RHVoice",0,KEY_QUERY_VALUE,&k)==ERROR_SUCCESS)
            {
              wchar_t buff[MAX_PATH+1];
              DWORD size=MAX_PATH*sizeof(wchar_t);
              DWORD type=0;
              if(RegQueryValueEx(k,L"data_path",0,&type,(BYTE*)buff,&size)==ERROR_SUCCESS)
                {
                  if(type==REG_SZ)
                    {
                      buff[size/sizeof(wchar_t)]=L'\0';
                      data_path_w.assign(buff);
                    }
                }
              RegCloseKey(k);
            }
          if(data_path_w.empty()) return E_FAIL;
          string data_path=wstring_to_string(data_path_w);
          string cfg_path;
          wchar_t appdata_path[MAX_PATH];
          if(SHGetFolderPath(NULL,CSIDL_APPDATA,NULL,0,appdata_path)==S_OK)
            {
              cfg_path.assign(wstring_to_string(appdata_path));
              if(*cfg_path.rbegin()!='\\') cfg_path+='\\';
              cfg_path+="RHVoice";
            }
          EnterCriticalSection(&init_mutex);
          try
            {
              if(sample_rate==0)
                {
                  sample_rate=RHVoice_initialize(data_path.c_str(),TTSTask::callback,cfg_path.empty()?NULL:cfg_path.c_str(),0);
                if(sample_rate>0)
                  CSpTTSEngineImpl::TTSTask::initialize();
                }
              if(sample_rate==0) result=E_FAIL;
            }
          catch(...)
            {
              LeaveCriticalSection(&init_mutex);
              throw;
            }
          LeaveCriticalSection(&init_mutex);
        }
      if(result==S_OK)
        {
          EnterCriticalSection(&object_token_mutex);
          try
            {
              object_token=pToken;
            }
          catch(...)
            {
              LeaveCriticalSection(&object_token_mutex);
              throw;
            }
          LeaveCriticalSection(&object_token_mutex);
        }
      if(SUCCEEDED(pToken->GetStringValue(L"voice",&name)))
        {
          try
            {
              if(RHVoice_find_voice(wstring_to_string(name).c_str()))
                voice.assign(name);
            }
          catch(...) {}
          CoTaskMemFree(name);
          name=NULL;
        }
      if(SUCCEEDED(pToken->GetStringValue(L"variant",&name)))
        {
          try
            {
              int id=RHVoice_find_variant(wstring_to_string(name).c_str());
              if(id>0) variant=id;
            }
          catch(...) {}
          CoTaskMemFree(name);
          name=NULL;
        }
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
  HRESULT result=E_FAIL;
  try
    {
      *ppToken=NULL;
      EnterCriticalSection(&object_token_mutex);
      if(object_token)
        {
          object_token.AddRef();
          *ppToken=object_token.GetInterfacePtr();
          result=S_OK;
        }
      LeaveCriticalSection(&object_token_mutex);
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

float CSpTTSEngineImpl::TTSTask::rate_table[21]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
float CSpTTSEngineImpl::TTSTask::pitch_table[49]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
float CSpTTSEngineImpl::TTSTask::default_native_rate=1.0;
float CSpTTSEngineImpl::TTSTask::default_native_pitch=1.0;
float CSpTTSEngineImpl::TTSTask::max_native_volume=2.0;

CSpTTSEngineImpl::TTSTask::TTSTask(const SPVTEXTFRAG *pTextFragList,ISpTTSEngineSite *pOutputSite,wstring voice_name,int variant_id,bool speak_punctuation) :
  message(NULL),
  audio_bytes(0),
  out(pOutputSite,true),
  current_sentence_number(0),
  skipping(false),
  sentence_count(0),
  voice(voice_name),
  variant(variant_id),
  punctuation_mode(speak_punctuation)
{
  generate_ssml(pTextFragList);
  message=RHVoice_new_message_utf16(reinterpret_cast<const uint16_t*>(ssml.data()),ssml.size(),1);
  if(message==NULL)
    throw runtime_error("Message creation failed");
  RHVoice_set_message_pitch(message,default_native_pitch);
  sentence_count=RHVoice_get_sentence_count(message);
  RHVoice_set_user_data(message,this);
}

CSpTTSEngineImpl::TTSTask::~TTSTask()
{
  if(message!=NULL) RHVoice_delete_message(message);
}

void CSpTTSEngineImpl::TTSTask::operator()()
{
  SPVSKIPTYPE skip_type;
  long count;
  int prev_sentence_number,next_sentence_number;
  set_rate();
  set_volume();
  RHVoice_speak(message);
  while(skipping)
    {
      skipping=false;
      if(SUCCEEDED(out->GetSkipInfo(&skip_type,&count)))
        {
          switch(skip_type)
            {
            case SPVST_SENTENCE:
              prev_sentence_number=(current_sentence_number==0)?1:current_sentence_number;
              next_sentence_number=prev_sentence_number+count;
              if(sentence_count==0)
                out->CompleteSkip(0);
              else if(next_sentence_number>sentence_count)
                out->CompleteSkip(sentence_count-prev_sentence_number);
                else if(next_sentence_number<=0)
                  out->CompleteSkip(-(prev_sentence_number-1));
                else
                  {
                    out->CompleteSkip(next_sentence_number-prev_sentence_number);
                    RHVoice_position p;
                    p.type=RHVoice_position_sentence;
                    p.info.number=next_sentence_number;
                    if(RHVoice_set_position(message,&p))
                      RHVoice_speak(message);
                  }
              break;
            default:
              out->CompleteSkip(0);
              break;
            }
        }
    }
}

void CSpTTSEngineImpl::TTSTask::initialize()
{
  default_native_rate=RHVoice_get_default_rate();
  default_native_pitch=RHVoice_get_default_pitch();
  max_native_volume=RHVoice_get_max_volume();
  int i;
  float p=0.1;
  float m=max(RHVoice_get_max_rate()/default_native_rate,default_native_rate/RHVoice_get_min_rate());
  float f=pow(m,p);
  rate_table[10]=1;
  for(i=1;i<=10;i++)
    {
      rate_table[10+i]=rate_table[9+i]*f;
      rate_table[10-i]=rate_table[11-i]/f;
    }
  m=max(RHVoice_get_max_pitch()/default_native_pitch,default_native_pitch/RHVoice_get_min_pitch());
  p=1.0/24.0;
  f=pow(m,p);
  pitch_table[24]=1;
  for(i=1;i<=24;i++)
    {
      pitch_table[24+i]=pitch_table[23+i]*f;
      pitch_table[24-i]=pitch_table[25-i]/f;
    }
}

float CSpTTSEngineImpl::TTSTask::get_pitch_factor(int value)
{
  return pitch_table[max(-24,min(24,value))+24];
}

float CSpTTSEngineImpl::TTSTask::get_rate_factor(int value)
{
  return rate_table[max(-10,min(10,value))+10];
}

void CSpTTSEngineImpl::TTSTask::set_rate()
{
  if(message==NULL) return;
  long rate=0;
  out->GetRate(&rate);
  RHVoice_set_message_rate(message,default_native_rate*get_rate_factor(rate));
}

float CSpTTSEngineImpl::TTSTask::get_volume_factor(unsigned int value)
{
  return (value>100)?1.0:(static_cast<float>(value)/100.0);
}

void CSpTTSEngineImpl::TTSTask::set_volume()
{
  if(message==NULL) return;
  unsigned short volume=0;
  out->GetVolume(&volume);
  RHVoice_set_message_volume(message,max_native_volume*get_volume_factor(volume));
}

void CSpTTSEngineImpl::TTSTask::write_text_to_stream(wostringstream& s,const wchar_t *text,size_t len)
{
  for(size_t i=0;i<len;i++)
    {
      if(text[i]==L'<')
        s << L"&#60;";
      else if(text[i]==L'&')
        s << L"&#38;";
      else if(text[i]==L'"')
        s << L"&#34;";
      else if(text[i]==L'\r')
    s << L"&#13;";
      else if((text[i]<32)&&(text[i]!=L'\t')&&(text[i]!=L'\n'))
        s.put(L' ');
      else
        s.put(text[i]);
    }
}

void CSpTTSEngineImpl::TTSTask::generate_ssml(const SPVTEXTFRAG *frags)
{
  frag_map.clear();
  wostringstream s;
  s.imbue(locale::classic());
  s.exceptions(wostringstream::failbit|wostringstream::badbit);
  s << L"<speak>";
  s << L"<voice";
  if(!voice.empty())
    {
      s << L" name=\"";
      s << voice;
      s << L"\"";
    }
  s << L" variant=\"";
  s << variant;
  s << L"\">";
  if(punctuation_mode)
    s << L"<tts:style field=\"punctuation\" mode=\"all\">";
  for(const SPVTEXTFRAG *frag=frags;frag;frag=frag->pNext)
    {
      switch(frag->State.eAction)
        {
        case SPVA_Speak:
        case SPVA_Pronounce:
        case SPVA_SpellOut:
          if(frag->ulTextLen>0)
            {
              s << L"<prosody rate=\"";
              s << get_rate_factor(frag->State.RateAdj);
              s << L"\" pitch=\"";
              s << 100.0*get_pitch_factor(frag->State.PitchAdj.MiddleAdj);
              s << L"%\" volume=\"";
              s << ((frag->State.Volume>100)?100:frag->State.Volume);
              s << L"%\">";
              if(frag->State.eAction==SPVA_SpellOut)
                s << L"<say-as interpret-as=\"characters\">";
              frag_map[s.tellp()]=frag;
              write_text_to_stream(s,frag->pTextStart,frag->ulTextLen);
              if(frag->State.eAction==SPVA_SpellOut)
                s << L"</say-as>";
              s << L"</prosody>";
            }
          break;
        case SPVA_Silence:
          s << L"<break time=\"";
          s << frag->State.SilenceMSecs;
          s << L"ms\"/>";
          break;
        case SPVA_Bookmark:
          s << L"<mark name=\"";
          write_text_to_stream(s,frag->pTextStart,frag->ulTextLen);
          s << L"\"/>";
          break;
        default:
          break;
        }
    }
  if(punctuation_mode)
    s << L"</tts:style>";
  s << L"</voice>";
  s << L"</speak>";
  ssml.assign(s.str());
}

wstring::const_iterator CSpTTSEngineImpl::TTSTask::skip_unichars(wstring::const_iterator first,size_t n)
{
  size_t i;
  wstring::const_iterator it;
  for(i=0,it=first;(it!=ssml.end())&&(i<n);it=skip_unichar(it),i++);
  return it;
}

unsigned long CSpTTSEngineImpl::TTSTask::convert_position(wstring::const_iterator ssml_pos)
{
  map<size_t,const SPVTEXTFRAG*>::const_iterator it=frag_map.lower_bound(ssml_pos-ssml.begin());
  if((it==frag_map.end())||
     (it->first>(ssml_pos-ssml.begin())))
    it--;
  wstring::const_iterator frag_start=ssml.begin()+it->first;
  unsigned long real_pos=it->second->ulTextSrcOffset+((ssml_pos-frag_start)-4*count(frag_start,ssml_pos,L'&'));
  return real_pos;
}

int CSpTTSEngineImpl::TTSTask::callback(const short *samples,int num_samples,const RHVoice_event *events,int num_events,RHVoice_message message)
{
  int result=1;
  try
    {
      CSpTTSEngineImpl::TTSTask *obj=static_cast<CSpTTSEngineImpl::TTSTask*>(RHVoice_get_user_data(message));
      result=obj->real_callback(samples,num_samples,events,num_events);
    }
  catch(...)
    {
      result=0;
    }
  return result;
}

int CSpTTSEngineImpl::TTSTask::real_callback(const short *samples,int num_samples,const RHVoice_event *events,int num_events)
{
  int result=1;
  try
    {
      DWORD a=out->GetActions();
      if(a&SPVES_ABORT) return 0;
      if(a&SPVES_RATE) set_rate();
      if(a&SPVES_VOLUME) set_volume();
      if(a&SPVES_SKIP)
        {
          skipping=true;
          return 0;
        }
      SPEVENT e;
      e.ulStreamNum=0;
      unsigned long bytes_written=0;
      wstring::const_iterator start,end;
      unsigned long real_start,real_end;
      long n;
      wstring m;
      wistringstream s;
      s.exceptions(wistringstream::failbit|wistringstream::badbit);
      for(int i=0;i<num_events;i++)
        {
          if(events[i].type==RHVoice_event_mark)
            {
              e.eEventId=SPEI_TTS_BOOKMARK;
              e.elParamType=SPET_LPARAM_IS_STRING;
            }
          else if(events[i].type==RHVoice_event_word_start)
            {
              e.eEventId=SPEI_WORD_BOUNDARY;
              e.elParamType=SPET_LPARAM_IS_UNDEFINED;
            }
          else if(events[i].type==RHVoice_event_sentence_start)
            {
              e.eEventId=SPEI_SENTENCE_BOUNDARY;
              e.elParamType=SPET_LPARAM_IS_UNDEFINED;
              current_sentence_number=events[i].id.number;
            }
          else continue;
          e.ullAudioStreamOffset=audio_bytes+events[i].audio_position*sizeof(short);
          if(events[i].type==RHVoice_event_mark)
            {
              m.assign(string_to_wstring(events[i].id.name));
              e.lParam=(LPARAM)m.c_str();
              s.str(m);
              try
                {
                  s >> n;
                }
              catch(...)
                {
                  n=0;
                }
              s.clear();
              e.wParam=n;
            }
          else
            {
              start=skip_unichars(ssml.begin(),events[i].text_position-1);
              end=skip_unichars(start,events[i].text_length);
              real_start=convert_position(start);
              real_end=convert_position(end);
              e.wParam=real_end-real_start;
              e.lParam=real_start;
            }
          out->AddEvents(&e,1);
        }
      if(samples!=NULL)
      {
        if(FAILED(out->Write(samples,num_samples*sizeof(short),&bytes_written)))
          result=0;
        audio_bytes+=bytes_written;
      }
    }
  catch(...)
    {
      result=0;
    }
  return result;
}

int CSpTTSEngineImpl::get_sample_rate()
{
  int r=0;
  EnterCriticalSection(&init_mutex);
  r=sample_rate;
  LeaveCriticalSection(&init_mutex);
  return r;
}
