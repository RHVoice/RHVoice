/* Copyright (C) 2012, 2013, 2019  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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
    const double SpeakImpl::rate_table[]={0.333333,0.372041,0.415244,0.463463,0.517282,0.577350,0.644394,0.719223,0.802742,0.895958,1.000000,
                                          1.116123,1.245731,1.390389,1.551846,1.732051,1.933182,2.157669,2.408225,2.687875,3.000000};
    const double SpeakImpl::pitch_table[]={0.500000,0.514651,0.529732,0.545254,0.561231,0.577676,0.594604,0.612027,0.629961,0.648420,0.667420,0.686977,
                                           0.707107,0.727827,0.749154,0.771105,0.793701,0.816958,0.840896,0.865537,0.890899,0.917004,0.943874,0.971532,1.000000,
                                           1.029302,1.059463,1.090508,1.122462,1.155353,1.189207,1.224054,1.259921,1.296840,1.334840,1.373954,1.414214,
                                           1.455653,1.498307,1.542211,1.587401,1.633915,1.681793,1.731073,1.781797,1.834008,1.887749,1.943064,2.000000};

    SpeakImpl::SpeakImpl(const init_params& p):
      caller(p.caller),
      doc(get_engine(),p.profile),
      bytes_written(0)
    {
      doc.speech_settings.relative.rate=get_rate();
      doc.speech_settings.absolute.volume=1;
      doc.speech_settings.relative.volume=get_volume();
      for(const SPVTEXTFRAG* frag=p.input;frag;frag=frag->pNext)
        {
          text_iterator text_start(frag,0);
          text_iterator text_end(frag,frag->ulTextLen);
          tts_markup markup;
          switch(frag->State.eAction)
            {
            case SPVA_Speak:
            case SPVA_SpellOut:
              markup.prosody.rate=convert_rate(frag->State.RateAdj);
              markup.prosody.pitch=convert_pitch(frag->State.PitchAdj.MiddleAdj);
              markup.prosody.volume=convert_volume(frag->State.Volume);
              if(frag->State.eAction==SPVA_SpellOut)
                markup.say_as=content_chars;
              doc.add_text(text_start,text_end,markup);
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
      if(actions&SPVES_RATE)
        doc.speech_settings.relative.rate=get_rate();
      if(actions&SPVES_VOLUME)
        doc.speech_settings.relative.volume=get_volume();
      return true;
    }

    bool SpeakImpl::play_speech(const short* samples,std::size_t count)
    {
      const BYTE *ptr=reinterpret_cast<const BYTE*>(samples);
      ULONG size=count*sizeof(short);
      ULONG remaining=size;
      ULONG written=0;
      while(remaining>0)
        {
          if(!check_actions())
            return false;
          written=remaining;
          if(FAILED(caller->Write(ptr,remaining,&written)))
            return false;
          if(written>remaining)
            return false;
          bytes_written+=written;
          remaining-=written;
          ptr+=written;
        }
      return true;
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
