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

#ifndef RHVOICE_SD_SPEAK_HPP
#define RHVOICE_SD_SPEAK_HPP

#include "command.hpp"
#include "tts_message.hpp"

namespace RHVoice
{
  namespace sd
  {
    namespace cmd
    {
      class speak: public command
      {
      public:
        explicit speak(tts_message_type type):
          message_type(type)
        {
        }

      private:
        bool is_valid() const;
        action_t execute();
        bool get_message(tts_message& msg) const;

        tts_message_type message_type;
      };
    }
  }
}
#endif
