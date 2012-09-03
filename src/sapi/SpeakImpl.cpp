/* Copyright (C) 2012  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <sstream>
#include <locale>
#include "core/tts_markup.hpp"
#include "utils.hpp"
#include "text_iterator.hpp"
#include "SpeakImpl.hpp"

namespace RHVoice
{
  namespace sapi
  {
    SpeakImpl::SpeakImpl(const init_params& p):
      caller(p.caller),
      doc(get_engine(),p.document_params),
      bytes_written(0)
    {
      for(const SPVTEXTFRAG* frag=p.input;frag;frag=frag->pNext)
        {
          text_iterator text_start(frag,0);
          text_iterator text_end(frag,frag->ulTextLen);
          switch(frag->State.eAction)
            {
            case SPVA_Speak:
            case SPVA_SpellOut:
              doc.add_text(text_start,text_end);
              break;
            case SPVA_Bookmark:
              doc.add_mark(utils::wstring_to_string(frag->pTextStart,frag->ulTextLen));
              break;
            default:
              break;
            }
        }
      doc.set_owner(*this);
    }

    void SpeakImpl::operator()()
    {
      doc.synthesize();
    }

    event_mask SpeakImpl::get_supported_events() const
    {
      event_mask mask=0;
      ULONGLONG event_interest;
      if(SUCCEEDED(const_cast<ISpTTSEngineSite*>(caller)->GetEventInterest(&event_interest)))
        {
          if(event_interest&SPEI_WORD_BOUNDARY)
            mask|=event_word_starts;
          if(event_interest&SPEI_SENTENCE_BOUNDARY)
            mask|=event_sentence_starts;
          if(event_interest&SPEI_TTS_BOOKMARK)
            mask|=event_mark;
        }
      return mask;
    }

    bool SpeakImpl::check_actions()
    {
      DWORD actions=caller->GetActions();
      if(actions&SPVES_ABORT)
        return false;
      if(actions&SPVES_SKIP)
        {
          caller->CompleteSkip(0);
          return false;
        }
      return true;
    }

    bool SpeakImpl::play_speech(const short* samples,std::size_t count)
    {
      if(check_actions())
        {
          ULONG size=count*sizeof(short);
          ULONG written=0;
          if(FAILED(caller->Write(samples,size,&written)))
            return false;
          if(size!=written)
            return false;
          bytes_written+=written;
          return true;
        }
      else
        return false;      
    }
    bool SpeakImpl::process_mark(const std::string& name)
    {
      if(check_actions())
        {
          std::wstring wname(utils::string_to_wstring(name));
          std::wistringstream s(wname);
          s.imbue(std::locale::classic());
          long as_number;
          if(!(s >> as_number))
            as_number=0;
          SPEVENT e;
          e.eEventId=SPEI_TTS_BOOKMARK;
          e.elParamType=SPET_LPARAM_IS_STRING;
          e.ulStreamNum=0;
          e.ullAudioStreamOffset=bytes_written;
          e.wParam=as_number;
          e.lParam=reinterpret_cast<LPARAM>(wname.c_str());
          return (SUCCEEDED(caller->AddEvents(&e,1)));
        }
      else
        return false;
    }

    bool SpeakImpl::queue_boundary_event(bool is_word,std::size_t position,std::size_t length)
    {
      SPEVENT e;
      e.eEventId=is_word?SPEI_WORD_BOUNDARY:SPEI_SENTENCE_BOUNDARY;
      e.elParamType=SPET_LPARAM_IS_UNDEFINED;
      e.ulStreamNum=0;
      e.ullAudioStreamOffset=bytes_written;
      e.wParam=length;
      e.lParam=position;
      return (SUCCEEDED(caller->AddEvents(&e,1)));
    }

    bool SpeakImpl::word_starts(std::size_t position,std::size_t length)
    {
      return (check_actions()&&queue_boundary_event(true,position,length));
    }

    bool SpeakImpl::sentence_starts(std::size_t position,std::size_t length)
    {
      return (check_actions()&&queue_boundary_event(false,position,length));
    }
  }
}
