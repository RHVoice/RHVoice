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

#ifndef RHVOICE_SD_COMMAND_HPP
#define RHVOICE_SD_COMMAND_HPP


#include "base.hpp"
#include "speech_synthesizer.hpp"

namespace RHVoice
{
  namespace sd
  {
    enum action_t
      {
        action_quit=0,
        action_abort=1,
        action_continue=2
      };

    class command: public base
    {
    public:
      virtual ~command()
      {
      }

      action_t run();

    protected:
      static speech_synthesizer synthesizer;

    private:
      virtual bool is_valid() const
      {
        return true;
      }

      virtual action_t execute()=0;
    };

    typedef std::shared_ptr<command> cmd_ptr;
  }
}
#endif
