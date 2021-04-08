/* Copyright (C) 2012, 2013  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include "utf8.h"
#include "state.hpp"
#include "speak.hpp"
#include "io.hpp"

namespace RHVoice
{
  namespace sd
  {
    namespace cmd
    {
      bool speak::get_message(tts_message& msg) const
      {
        reply r;
        if(current_voice.empty())
          {
            r("300 ERROR NO VOICE IS SET");
            return false;
          }
        r("202 OK RECEIVING MESSAGE");
        msg.content.clear();
        std::size_t nlines=0;
        for(std::string line=getline();line!=".";++nlines,line=getline())
          {
            if(line=="..")
              msg.content.append(".");
              else
                msg.content.append(line);
            msg.content.push_back('\n');
          }
        if(!msg.content.empty())
          msg.content.erase(msg.content.end()-1);
        logger::log(5,"Received message\n",msg.content);
        if(msg.content.empty())
          {
            r("305 ERROR EMPTY MESSAGE");
            return false;
          }
        if((message_type!=tts_message_text)&&(nlines>1))
          {
            r("305 ERROR UNEXPECTED MULTILINE INPUT");
            return false;
          }
        if((message_type==tts_message_char)&&(msg.content=="space"))
          msg.content=" ";
        try
          {
            std::size_t len=utf8::distance(msg.content.begin(),msg.content.end());
            if((message_type==tts_message_char)&&(len>1))
              {
                r("305 ERROR UNEXPECTED MULTI CHARACTER INPUT");
                return false;
              }
          }
        catch(const utf8::exception& e)
          {
            r("305 ERROR INVALID UTF-8 STRING");
            return false;
          }
        msg.type=message_type;
        msg.profile=current_voice;
        msg.set_synthesis_params(tts_settings);
        r("200 OK SPEAKING");
        return true;
      }

      bool speak::is_valid() const
      {
        return ((!(tts_engine == nullptr))&&
                playback_stream.is_initialized()&&
                (!state::Test(state::speaking)));
      }

      action_t speak::execute()
      {
        tts_message msg;
        if(get_message(msg))
          {
            state::Set(state::speaking);
            synthesizer.send_message(msg);
          }
        return action_continue;
      }
    }
  }
}
