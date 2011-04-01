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
using std::count;
using std::runtime_error;

CRITICAL_SECTION init_mutex;

float CSpTTSEngineImpl::TTSTask::pitch_table[21]={0.749154,
                                                  0.771105,
                                                  0.793701,
                                                  0.816958,
                                                  0.840896,
                                                  0.865537,
                                                  0.890899,
                                                  0.917004,
                                                  0.943874,
                                                  0.971532,
                                                  1.000000,
                                                  1.029302,
                                                  1.059463,
                                                  1.090508,
                                                  1.122462,
                                                  1.155353,
                                                  1.189207,
                                                  1.224054,
                                                  1.259921,
                                                  1.296840,
                                                  1.334840};
float CSpTTSEngineImpl::TTSTask::rate_table[21]={0.333333,
                                                 0.372041,
                                                 0.415244,
                                                 0.463463,
                                                 0.517282,
                                                 0.577350,
                                                 0.644394,
                                                 0.719223,
                                                 0.802742,
                                                 0.895958,
                                                 1.000000,
                                                 1.116123,
                                                 1.245731,
                                                 1.390389,
                                                 1.551846,
                                                 1.732051,
                                                 1.933182,
                                                 2.157669,
                                                 2.408225,
                                                 2.687875,
                                                 3.000000};
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
      TTSTask t(pTextFragList,pOutputSite,variant);
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
  DWORD v=1;
  try
    {
      if(pToken==NULL)
        return E_INVALIDARG;
      if(get_sample_rate()==0)
        {
          wchar_t *voice_path_w=NULL;
          if(FAILED(pToken->GetStringValue(L"VoicePath",&voice_path_w)))
            return E_FAIL;
          string voice_path;
          try
            {
              voice_path=wstring_to_string(voice_path_w);
            }
          catch(...)
            {
              CoTaskMemFree(voice_path_w);
              throw;
            }
          CoTaskMemFree(voice_path_w);
          voice_path_w=NULL;
          EnterCriticalSection(&init_mutex);
          try
            {
              if(sample_rate==0)
                sample_rate=RHVoice_initialize(voice_path.c_str(),TTSTask::callback);
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
      if(SUCCEEDED(pToken->GetDWORD(L"Variant",&v)))
        {
          if((v>0)&&(v<3))
            variant=v;
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

CSpTTSEngineImpl::TTSTask::TTSTask(const SPVTEXTFRAG *pTextFragList,ISpTTSEngineSite *pOutputSite,DWORD dwVariant) :
  message(NULL),
  audio_bytes(0),
  out(pOutputSite,true),
  current_sentence_number(0),
  skipping(false),
  sentence_count(0),
  variant(dwVariant)
{
  generate_ssml(pTextFragList);
  message=RHVoice_new_message_utf16(reinterpret_cast<const uint16_t*>(ssml.data()),ssml.size(),1);
  if(message==NULL)
    throw runtime_error("Message creation failed");
  RHVoice_set_message_pitch(message,50);
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

float CSpTTSEngineImpl::TTSTask::get_pitch_factor(int value)
{
  return pitch_table[max(-10,min(10,value))+10];
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
  RHVoice_set_message_rate(message,20.0*get_rate_factor(rate));
}

float CSpTTSEngineImpl::TTSTask::convert_volume(unsigned int value)
{
  return value>100?100:value;
}

void CSpTTSEngineImpl::TTSTask::set_volume()
{
  if(message==NULL) return;
  unsigned short volume=0;
  out->GetVolume(&volume);
  RHVoice_set_message_volume(message,convert_volume(volume));
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
  s << L"<voice variant=\"";
  s << variant;
  s << L"\">";
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
              s << 100.0*get_rate_factor(frag->State.RateAdj);
              s << L"%\" pitch=\"";
              s << 100.0*get_pitch_factor(frag->State.PitchAdj.MiddleAdj);
              s << L"%\" volume=\"";
              s << convert_volume(frag->State.Volume);
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
