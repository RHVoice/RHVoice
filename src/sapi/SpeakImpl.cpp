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

#include <stdexcept>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/lexical_cast.hpp>
#include "SpeakImpl.h"
#include "util.h"

using std::wstring;
using std::pair;
using std::min;
using std::runtime_error;
using std::vector;
using std::fill_n;
using std::copy;
using std::back_inserter;
using boost::shared_ptr;
using boost::optional;
using boost::numeric_cast;
using boost::lexical_cast;
using boost::bad_lexical_cast;

SpeakImpl::SpeakImpl(const SPVTEXTFRAG *pTextFragList,ISpTTSEngineSite *pOutputSite,cst_voice *Voice) :
  sentence_start(pTextFragList,0),
  sentence_end(pTextFragList,0),
  output(pOutputSite),
  voice(Voice),
  whitespace_chars(string_to_wstring(feat_string(Voice->features,"text_whitespace"))),
  prepunc_chars(string_to_wstring(feat_string(Voice->features,"text_prepunctuation"))),
  postpunc_chars(string_to_wstring(feat_string(Voice->features,"text_postpunctuation"))),
  audio_bytes_written(0),
  actions(output->GetActions()),
  stopped(false)
{
}

HRESULT SpeakImpl::operator()()
{
  while(!is_stopped())
    {
      if(!next_sentence())
        break;
      synth_sentence();
    }
  return S_OK;
}

int SpeakImpl::callback(const short *samples,int nsamples,cst_item *seg,int pos_in_seg)
{
  if(samples==NULL)
    return 1;
  SpeakImpl *sp=static_cast<SpeakImpl*>(val_userdata(feat_val(item_utt(seg)->features,"this")));
  return sp->real_callback(samples,nsamples,seg,pos_in_seg);
}

int SpeakImpl::real_callback(const short *samples,int nsamples,cst_item *seg,int pos_in_seg)
{
  if(is_interrupted())
    return 0;
  const cst_item *s=seg;
  const cst_item *t1,*t2;
  int n=0;
  int p=pos_in_seg;
  int ns=0;
  while(n<nsamples)
    {
      if((p==0)&&
         item_as(s,"Transcription")&&
         !item_prev(item_as(s,"Transcription")))
        {
          t2=path_to_item(s,"R:Transcription.parent.R:Token.parent");
          t1=path_to_item(s,"R:Transcription.parent.p.R:Token.parent");
          if(t1!=t2)
            {
              if(t1==NULL)
                t1=relation_head(utt_relation(item_utt(s),"Token"));
              if(t1!=t2)
                process_bookmarks_and_silences(t1,t2);
              events.push_back(event_ptr(new word_event(t2,calculate_event_audio_offset())));
            }
        }
      ns=min(nsamples-n,item_feat_int(s,"nsamples")-p);
      queue_audio(samples+n,ns);
      n+=ns;
      s=item_next(s);
      if(s==NULL)
        break;
      p=0;
    }
  write_queued_data();
  return 1;
}

optional<SpeakImpl::token> SpeakImpl::get_token()
{
  optional<token> r;
  if(sentence_end.first==NULL)
    return r;
  token t;
  t.start=sentence_end;
  wstring s;
  size_t i,j;
  for(pos p=t.start;p.first;p=pos(p.first->pNext,0))
    {
      if((p.first->State.eAction!=SPVA_Speak)&&
         (p.first->State.eAction!=SPVA_SpellOut))
        continue;
      if(p.second>=p.first->ulTextLen)
        continue;
      s.assign(p.first->pTextStart,p.first->ulTextLen);
      i=s.find_first_not_of(whitespace_chars,p.second);
      if(i!=p.second)
        t.whitespace.append(s,p.second,(i==wstring::npos?s.length():i)-p.second);
      if(i!=wstring::npos)
        {
          t.text_start=pos(p.first,i);
          j=s.find_first_of(whitespace_chars,i);
          t.text.assign(s,i,(j==wstring::npos?s.length():j)-i);
          t.end=j==wstring::npos?pos(p.first->pNext,0):pos(p.first,j);
          break;
        }
    }
  if(!t.text.empty())
    {
      i=t.text.find_first_not_of(prepunc_chars);
      if(i!=0)
        t.prepunctuation.assign(t.text,0,i==wstring::npos?t.text.length():i);
      if(i!=wstring::npos)
        {
          s=t.text.substr(i);
          j=s.find_last_not_of(postpunc_chars);
          if(j==wstring::npos)
            t.postpunctuation=s;
          else if(j==s.length()-1)
            t.name=s;
          else
            {
              t.name=s.substr(0,j+1);
              t.postpunctuation=s.substr(j+1);
            }
        }
      r=t;
    }
  return r;
}

void SpeakImpl::add_token(const token& t)
{
  cst_relation *t_rel=utt_relation(cur_utt.get(),"Token");
  cst_item *t_item=relation_append(t_rel,NULL);
  item_set_string(t_item,"whitespace",wstring_to_string(t.whitespace).c_str());
  item_set_string(t_item,"name",wstring_to_string(t.name).c_str());
  item_set_string(t_item,"prepunctuation",wstring_to_string(t.prepunctuation).c_str());
  item_set_string(t_item,"punc",wstring_to_string(t.postpunctuation).c_str());
  item_set(t_item,"text_frag",userdata_val(t.text_start.first));
  item_set_int(t_item,"pos_in_text_frag",numeric_cast<int>(t.text_start.second));
  item_set_int(t_item,"length_w",numeric_cast<int>(t.text.length()));
}

bool SpeakImpl::is_end_of_sentence(const token& next_token)
{
  if(!cur_token)
    return false;
  if(next_token.whitespace.find(L'\n')!=next_token.whitespace.rfind(L'\n'))
    return true;
  if(cur_token->postpunctuation.empty())
    return false;
  if(cur_token->name.empty())
    return false;
  if(*(cur_token->postpunctuation.rbegin())==L';')
    return true;
  if(*(cur_token->postpunctuation.rbegin())==L':')
    {
      if((next_token.whitespace.find(L'\n')!=wstring::npos)&&
         (next_token.text[0]==L'-'))
        return true;
      if(next_token.text[0]==L'\"')
        return true;
      return false;
    }
  if(cur_token->postpunctuation.find_last_of(L"?!")!=wstring::npos)
    {
      if(*(cur_token->postpunctuation.rbegin())==L'-')
        return false;
      if((*(cur_token->postpunctuation.rbegin())==L'\"')&&
         (next_token.text[0]==L'-')&&
         (next_token.whitespace.find(L'\n')==wstring::npos))
        return false;
      return true;
    }
  if(cur_token->postpunctuation.find(L'.')!=wstring::npos)
    {
      if(cur_token->postpunctuation.length()>1)
        return true;
      if(next_token.name.empty())
        return true;
      if(!is_cap_letter(next_token.name[0]))
        return false;
      if(cur_token->name.length()>1)
        return true;
      if(is_cap_letter(cur_token->name[0]))
        return false;
      return true;
    }
  return false;
}

bool SpeakImpl::next_sentence()
{
  if(sentence_end.first==NULL)
    return false;
  cur_utt.reset(new_utterance(),delete_utterance);
  feat_set(cur_utt->features,"this",userdata_val(this));
  utt_init(cur_utt.get(),voice);
  utt_relation_create(cur_utt.get(),"Token");
  sentence_start=sentence_end;
  if(cur_token)
    {
      add_token(*cur_token);
      sentence_end=cur_token->end;
    }
  optional<token> next_token;
  for(next_token=get_token();next_token;next_token=get_token())
    {
      if(is_end_of_sentence(*next_token))
        break;
      add_token(*next_token);
      cur_token=next_token;
      sentence_end=cur_token->end;
    }
  cur_token=next_token;
  if(!cur_token)
    sentence_end=pos(NULL,0);
  return true;
}

void SpeakImpl::queue_silence(ULONG ms)
{
  if(ms==0)
    return;
  ULONG nsamples=numeric_cast<ULONG>((get_param_int(voice->features,"sample_rate",16000)/1000)*ms);
  fill_n(back_inserter(audio),nsamples,0);
}

void SpeakImpl::queue_audio(const short *samples,int nsamples)
{
  if(samples==NULL)
    return;
  copy(samples,samples+nsamples,back_inserter(audio));
}

void SpeakImpl::process_bookmarks_and_silences(const cst_item *t1,const cst_item *t2)
{
  const SPVTEXTFRAG *f1=sentence_start.first;
  if(t1!=NULL)
    f1=static_cast<const SPVTEXTFRAG*>(val_userdata(item_feat(t1,"text_frag")));
  const SPVTEXTFRAG *f2=sentence_end.first;
  if(t2!=NULL)
    f2=static_cast<const SPVTEXTFRAG*>(val_userdata(item_feat(t2,"text_frag")));
  for(const SPVTEXTFRAG *f=f1;f!=f2;f=f->pNext)
    {
      switch(f->State.eAction)
        {
        case SPVA_Bookmark:
          events.push_back(event_ptr(new bookmark_event(wstring(f->pTextStart,f->ulTextLen),
                                                        calculate_event_audio_offset())));
          break;
        case SPVA_Silence:
          queue_silence(f->State.SilenceMSecs);
          break;
        default:
          break;
        }
    }
}

void SpeakImpl::write_queued_data()
{
  for(vector<event_ptr>::const_iterator it=events.begin();it!=events.end();++it)
    {
      output->AddEvents((*it)->get_sapi_event(),1);
    }
  events.clear();
  if(audio.empty())
    return;
  ULONG b=0;
  stopped=FAILED(output->Write(&audio[0],audio.size()*sizeof(short),&b));
  audio_bytes_written+=b;
  audio.clear();
}

ULONGLONG SpeakImpl::calculate_event_audio_offset()
{
  return audio_bytes_written+audio.size()*sizeof(short);
}

SpeakImpl::bookmark_event::bookmark_event(const wstring& name,ULONGLONG astream_offset) :
  bookmark(name)
{
  sapi_event.eEventId=SPEI_TTS_BOOKMARK;
  sapi_event.elParamType=SPET_LPARAM_IS_STRING;
  sapi_event.ullAudioStreamOffset=astream_offset;
  try
    {
      sapi_event.wParam=(WPARAM)lexical_cast<long>(bookmark);
    }
  catch(bad_lexical_cast&)
    {
      sapi_event.wParam=0;
    }
  sapi_event.lParam=reinterpret_cast<LPARAM>(bookmark.c_str());
}

SpeakImpl::sentence_event::sentence_event(cst_utterance *utt,ULONGLONG astream_offset)
{
  const cst_item *t1=relation_head(utt_relation(utt,"Token"));
  if(t1==NULL)
    throw runtime_error("Empty sentence");
  const cst_item *t2=relation_tail(utt_relation(utt,"Token"));
  const SPVTEXTFRAG *f1=static_cast<const SPVTEXTFRAG *>(val_userdata(item_feat(t1,"text_frag")));
  const SPVTEXTFRAG *f2=static_cast<const SPVTEXTFRAG *>(val_userdata(item_feat(t2,"text_frag")));
  ULONG start=f1->ulTextSrcOffset+item_feat_int(t1,"pos_in_text_frag");
  ULONG length=f2->ulTextSrcOffset+item_feat_int(t2,"pos_in_text_frag")-start+item_feat_int(t2,"length_w");
  sapi_event.eEventId=SPEI_SENTENCE_BOUNDARY;
  sapi_event.elParamType=SPET_LPARAM_IS_UNDEFINED;
  sapi_event.ullAudioStreamOffset=astream_offset;
  sapi_event.wParam=length;
  sapi_event.lParam=(LPARAM)start;
}

SpeakImpl::word_event::word_event(const cst_item *t,ULONGLONG astream_offset)
{
  const SPVTEXTFRAG *frag=static_cast<const SPVTEXTFRAG *>(val_userdata(item_feat(t,"text_frag")));
  ULONG start=frag->ulTextSrcOffset+item_feat_int(t,"pos_in_text_frag");
  ULONG length=item_feat_int(t,"length_w");
  sapi_event.eEventId=SPEI_WORD_BOUNDARY;
  sapi_event.elParamType=SPET_LPARAM_IS_UNDEFINED;
  sapi_event.ullAudioStreamOffset=astream_offset;
  sapi_event.wParam=length;
  sapi_event.lParam=(LPARAM)start;
}

DWORD SpeakImpl::get_actions()
{
  DWORD new_actions=output->GetActions();
  if(new_actions!=SPVES_CONTINUE)
    actions=new_actions;
  return actions;
}

bool SpeakImpl::is_stopped()
{
  return (stopped||(get_actions()&SPVES_ABORT));
}

bool SpeakImpl::is_skipping()
{
  return (get_actions()&SPVES_SKIP);
}

bool SpeakImpl::is_interrupted()
{
  return (is_stopped()||is_skipping());
}

void SpeakImpl::synth_sentence()
{
  audio.clear();
  events.clear();
  const cst_item *first_token=relation_head(utt_relation(cur_utt.get(),"Token"));
  if(first_token==NULL)
    {
      process_bookmarks_and_silences(NULL,NULL);
      write_queued_data();
      return;
    }
  process_bookmarks_and_silences(NULL,first_token);
  events.push_back(event_ptr(new sentence_event(cur_utt.get(),calculate_event_audio_offset())));
  utt_synth_tokens(cur_utt.get());
  if(is_interrupted())
    return;
  const cst_item *last_spoken_token=item_parent(item_as(relation_tail(utt_relation(cur_utt.get(),"Transcription")),"Token"));
  if(last_spoken_token!=NULL)
    process_bookmarks_and_silences(last_spoken_token,NULL);
  else
    process_bookmarks_and_silences(first_token,NULL);
  write_queued_data();
}
