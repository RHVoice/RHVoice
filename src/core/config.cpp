/* Copyright (C) 2012, 2014, 2018  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 2.1 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#include <exception>
#include "core/ini_parser.hpp"
#include "core/io.hpp"
#include "core/config.hpp"

namespace
{
  const std::string tag("config");
}

namespace RHVoice
{
  bool config::set(const std::string& name,const std::string& value)
  {
    registration_map::iterator it=registered_settings.find(name);
                if(it==registered_settings.end())
                  return false;
                else
                  return it->second->set_from_string(value);
  }

  bool config::reset(const std::string& name)
  {
    registration_map::iterator it=registered_settings.find(name);
                if(it==registered_settings.end())
                  return false;
                else
                  {
                    it->second->reset();
                    return true;
                  }
  }

  void config::reset()
  {
    for(registration_map::iterator it=registered_settings.begin();it!=registered_settings.end();++it)
      {
        it->second->reset();
      }
  }

  void config::load(const std::string& file_path)
  {
    try
      {
        logger->log(tag,RHVoice_log_level_info,"trying to load configuration file "+file_path);
        for(ini_parser p(file_path);!p.done();p.next())
          {
            if(p.get_section().empty())
              {
                logger->log(tag,RHVoice_log_level_trace,p.get_key()+"="+p.get_value());
                set(p.get_key(),p.get_value());
              }
          }
        logger->log(tag,RHVoice_log_level_info,"configuration file processed");
      }
    catch(const io::open_error& e)
      {
        logger->log(tag,RHVoice_log_level_warning,e.what());
      }
    catch(const std::exception& e)
      {
        logger->log(tag,RHVoice_log_level_warning,e.what());
        throw;
      }
  }
}
