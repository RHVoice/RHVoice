/* Copyright (C) 2012, 2018  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 2 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#include <exception>
#include <algorithm>
#include "core/document.hpp"
#include "io.hpp"
#include "speech_synthesizer.hpp"

namespace RHVoice
{
  namespace sd
  {
    bool speech_synthesizer::is_ssml(const tts_message& msg) const
    {
      if(msg.type!=tts_message_text)
        return false;
      auto end=str::utf8_string_end(msg.content);
      auto pos=std::find_if_not(str::utf8_string_begin(msg.content), end, &str::isspace);
      if(pos==end)
        return false;
      return (*pos=='<');
    }

    void speech_synthesizer::signal_stop()
    {
      threading::lock message_lock(message_mutex);
      status=status_stopped;
      message_available.signal();
    }

    void speech_synthesizer::stop()
    {
      if(!has_started())
        return;
      state::Set(state::stopped);
      signal_stop();
      player.stop();
      player.join();
      logger::log(2,"Stopped the playback thread");
    }

    void speech_synthesizer::send_message(const tts_message& msg)
    {
      threading::lock message_lock(message_mutex);
      if(status!=status_empty)
        return;
      current_message=msg;
      status=status_message;
      message_available.signal();
    }

    bool speech_synthesizer::get_message(tts_message& msg)
    {
      threading::lock message_lock(message_mutex);
      if(status==status_stopped)
        return false;
      while(status!=status_message)
        {
          message_available.wait(message_mutex);
          if(status==status_stopped)
            return false;
        }
      msg=current_message;
      status=status_empty;
      return true;
    }

    void speech_synthesizer::prepare()
    {
      player.start();
      logger::log(2,"Started the playback thread");
    }

    void speech_synthesizer::run()
    {
      try
        {
          tts_message msg;
          while(get_message(msg))
            {
              player.enqueue(synth_result_ptr(new start_of_speech));
              synthesize_message(msg);
              player.enqueue(synth_result_ptr(new end_of_speech));
        }
        }
      catch(const std::exception& e)
        {
          logger::log(2,"Fatal error in the synthesis thread: ",e.what());
        }
    }

    void speech_synthesizer::synthesize_message(const tts_message& msg)
    {
      try
        {
          std::unique_ptr<document> doc;
          switch(msg.type)
            {
            case tts_message_text:
              doc=is_ssml(msg)?document::create_from_ssml(tts_engine,msg.content.begin(),msg.content.end(),msg.profile):document::create_from_plain_text(tts_engine,msg.content.begin(),msg.content.end(),content_text,msg.profile);
              break;
            case tts_message_char:
              doc=document::create_from_plain_text(tts_engine,msg.content.begin(),msg.content.end(),content_char,msg.profile);
              break;
            case tts_message_key:
              doc=document::create_from_plain_text(tts_engine,msg.content.begin(),msg.content.end(),content_key,msg.profile);
              break;
            default:
              doc=document::create_from_plain_text(tts_engine,msg.content.begin(),msg.content.end(),content_text,msg.profile);
              break;
            }
          if(cancelled())
            return;
          doc->speech_settings.absolute.rate=msg.rate;
          doc->speech_settings.absolute.pitch=msg.pitch;
          doc->speech_settings.absolute.volume=msg.volume;
          doc->verbosity_settings.punctuation_mode=msg.punctuation_mode;
          doc->verbosity_settings.capitals_mode=msg.capitals_mode;
          doc->set_owner(*this);
          doc->synthesize();
        }
      catch(const exception& e)
        {
          logger::log(2,"Synthesis error: ",e.what());
        }
    }

    bool speech_synthesizer::process_mark(const std::string& name)
    {
      player.enqueue(synth_result_ptr(new index_mark(name)));
      return (!cancelled_or_pausing());
    }

    bool speech_synthesizer::play_speech(const short* samples,std::size_t count)
    {
      if(cancelled())
        return false;
      player.enqueue(synth_result_ptr(new speech_chunk(samples,count)));
      return true;
    }

    bool speech_synthesizer::set_sample_rate(int sample_rate)
    {
      if(cancelled())
        return true;
      player.enqueue(synth_result_ptr(new sample_rate_setting(sample_rate)));
      return true;
    }
  }
}
