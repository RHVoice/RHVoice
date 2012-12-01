/* Copyright (C) 2012  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include <exception>
#include "config_tts.hpp"
#include "io.hpp"

namespace RHVoice
{
  namespace sd
  {
    namespace cmd
    {
      config_tts::config_tts()
      {
        register_settings(tts_settings);
      }

      bool config_tts::is_valid() const
      {
        return !tts_engine.empty();
      }

      action_t config_tts::execute()
      {
        reply r;
        r("203 OK RECEIVING SETTINGS");
        read_settings();
        set_voice();
        if(current_voice.is_empty())
          r("303 ERROR NO VOICE HAS BEEN SET");
        else
          r("203 OK SETTINGS RECEIVED");
        return action_continue;
      }

      void config_tts::set_voice()
      {
        voice_description new_voice;
        if(tts_settings.voice_name.is_set())
          new_voice=voices[tts_settings.voice_name];
        else
          {
            if(tts_settings.language_code.is_set())
              {
                voice_description voice1,voice2,voice3;
                RHVoice_voice_gender gender=RHVoice_voice_gender_unknown;
                unsigned int index=0;
                voice_id_t id=tts_settings.voice_id;
                if(id>0)
                  {
                    gender=RHVoice_voice_gender_female;
                    index=id;
                  }
                else if(id<0)
                  {
                    gender=RHVoice_voice_gender_male;
                    index=-id;
                  }
                unsigned int count=0;
                for(voice_map::const_iterator it=voices.begin();it!=voices.end();++it)
                  {
                    if(it->second.is_monolingual()&&(it->second.get_language_code()==tts_settings.language_code.get()))
                      {
                        if(voice3.is_empty())
                          voice3=it->second;
                        if((gender!=RHVoice_voice_gender_unknown)&&(it->second.get_main_voice()->get_gender()==gender))
                          {
                            ++count;
                            if(voice2.is_empty())
                              voice2=it->second;
                            if(count==index)
                              voice1=it->second;
                          }
                      }
                  }
                if(voice1.is_empty())
                  {
                    if(voice2.is_empty())
                      {
                        if(!voice3.is_empty())
                          new_voice=voice3;
                      }
                    else
                      new_voice=voice2;
                  }
                else
                  new_voice=voice1;
              }
          }
        if(!new_voice.is_empty())
          {
            current_voice=new_voice;
            logger::log(4,"Will speak using voice ",current_voice.get_name());
          }
      }
    }
  }
}
