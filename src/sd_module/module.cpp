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

#include <exception>
#include "module.hpp"
#include "io.hpp"
#include "init.hpp"
#include "config_audio.hpp"
#include "config_tts.hpp"
#include "config_logger.hpp"
#include "list_voices.hpp"
#include "speak.hpp"
#include "stop.hpp"
#include "pause.hpp"
#include "quit.hpp"

namespace RHVoice
{
  namespace sd
  {
    module::module()
    {
      register_subcommand<cmd::init>("INIT");
      register_subcommand<cmd::config_audio>("AUDIO");
      register_subcommand<cmd::config_tts>("SET");
      register_subcommand<cmd::config_logger>("LOGLEVEL");
      register_subcommand<cmd::list_voices>("LIST VOICES");
      register_subcommand<cmd::speak>(tts_message_text,"SPEAK");
      register_subcommand<cmd::speak>(tts_message_char,"CHAR");
      register_subcommand<cmd::speak>(tts_message_key,"KEY");
      register_subcommand<cmd::speak>(tts_message_icon,"SOUND_ICON");
      register_subcommand<cmd::stop>("STOP");
      register_subcommand<cmd::pause>("PAUSE");
      register_subcommand<cmd::quit>("QUIT");
    }

    action_t module::execute()
    {
      logger::log(1,"Module has started");
      action_t action=action_continue;
      std::string line;
      while(action==action_continue)
        {
          try
            {
              line=getline();
              cmd_map::iterator it=subcommands.find(line);
              if(it==subcommands.end())
                {
                  logger::log(4,"Unknown command: ",line);
                  reply()("300 ERROR UNKNOWN COMMAND");
                }
              else
                {
                  logger::log(4,"Received command ",line);
                  action=it->second->run();
                }
            }
          catch(const std::exception& e)
            {
              logger::log(2,"aborting because of an unexpected error: ",e.what());
              action=action_abort;
            }
        }
      try
        {
          close();
        }
      catch(const std::exception& e)
        {
          logger::log(2,"Error while closing the module: ",e.what());
        }
      logger::log(1,"Exiting");
      return action;
    }

    void module::close()
    {
      logger::log(2,"Closing the module");
      if(synthesizer.has_started())
        {
          synthesizer.stop();
          synthesizer.join();
          logger::log(2,"Stopped the synthesis thread");
        }
      if(playback_stream.is_open())
        {
          playback_stream.close();
          logger::log(2,"Closed the playback stream");
        }
      if(tts_engine)
        {
          tts_engine.reset();
          logger::log(2,"Freed the engine");
        }
    }
  }
}
