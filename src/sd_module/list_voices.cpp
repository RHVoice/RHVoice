/* Copyright (C) 2012, 2013, 2018  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include <set>
#include "core/engine.hpp"
#include "core/voice_profile.hpp"
#include "list_voices.hpp"
#include "io.hpp"

namespace
{
#if ((SPD_MAJOR==0) && (SPD_MINOR<9))
  const char sep=' ';
#else
  const char sep='\t';
  #endif
}

namespace RHVoice
{
  namespace sd
  {
    namespace cmd
    {
      bool list_voices::is_valid() const
      {
        return !(tts_engine == nullptr);
      }

      action_t list_voices::execute()
      {
        const std::set<voice_profile>& profiles=tts_engine->get_voice_profiles();
        reply r;
        if(profiles.empty())
          r("400 NO VOICES FOUND");
          else
            {
              std::string voice_name,language_code;
              std::string dialect("none");
              for(std::set<voice_profile>::const_iterator it=profiles.begin();it!=profiles.end();++it)
                {
                  voice_name=it->get_name();
                  language_code=it->primary()->get_language()->get_alpha2_code();
                  if(language_code.empty())
                    language_code="none";
                  r("200-",voice_name,sep,language_code,sep,dialect);
                }
              r("200 OK VOICE LIST SENT");
            }
        return action_continue;
      }
    }
  }
}
