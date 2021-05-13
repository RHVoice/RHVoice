/* Copyright (C) 2012  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include "command.hpp"
#include "io.hpp"

namespace RHVoice
{
  namespace sd
  {
    speech_synthesizer command::synthesizer;

    action_t command::run()
    {
      if(is_valid())
        return execute();
      else
        {
          reply()("300 ERROR INVALID COMMAND");
          return action_continue;
        }
    }
  }
}
