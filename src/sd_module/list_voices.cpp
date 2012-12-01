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

#include "list_voices.hpp"
#include "io.hpp"

namespace RHVoice
{
  namespace sd
  {
    namespace cmd
    {
      bool list_voices::is_valid() const
      {
        return !tts_engine.empty();
      }

      action_t list_voices::execute()
      {
        reply r;
        if(voices.empty())
          r("400 NO VOICES FOUND");
          else
            {
              std::string voice_name,language_code;
              std::string dialect("none");
              for(voice_map::const_iterator it=voices.begin();it!=voices.end();++it)
                {
                  voice_name=it->second.get_name();
                  language_code=it->second.get_language_code();
                  if(language_code.empty())
                    language_code="none";
                  r("200-",voice_name," ",language_code," ",dialect);
                }
              r("200 OK VOICE LIST SENT");
            }
        return action_continue;
      }
    }
  }
}
