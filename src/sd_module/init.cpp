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

#include <exception>
#include <set>
#include "core/engine.hpp"
#include "core/voice_profile.hpp"
#include "init.hpp"
#include "io.hpp"

namespace RHVoice
{
  namespace sd
  {
    namespace cmd
    {
      bool init::is_valid() const
      {
        return tts_engine == nullptr;
      }

      action_t init::execute()
      {
        try
          {
            tts_engine=engine::create();
            logger::log(3,"Initialized the engine");
            register_languages();
            register_voices();
            synthesizer.start();
            logger::log(2,"Started the synthesis thread");
          }
        catch(const std::exception& e)
          {
            logger::log(2,"Initialization error: ",e.what());
            reply r;
            r("399-",e.what());
            r("399 ERROR CANNOT INITIALIZE MODULE");
            return action_abort;
          }
        reply r;
        r("299-RHVoice initialized");
        r("299 OK LOADED SUCCESSFULLY");
        return action_continue;
      }

      void init::register_languages()
      {
        std::string code;
        const language_list& languages=tts_engine->get_languages();
        for(language_list::const_iterator it=languages.begin();it!=languages.end();++it)
          {
            logger::log(4,it->get_name()," language supported");
            code=it->get_alpha2_code();
            if(code.empty())
              code=it->get_alpha3_code();
            if(!code.empty())
              tts_settings.language_code.define(code);
          }
      }

      void init::register_voices()
      {
        const std::set<voice_profile>& profiles=tts_engine->get_voice_profiles();
        for(std::set<voice_profile>::const_iterator it=profiles.begin();it!=profiles.end();++it)
          {
            logger::log(4,"Found ",it->get_name());
            tts_settings.voice_name.define(it->get_name());
          }
      }
    }
  }
}
