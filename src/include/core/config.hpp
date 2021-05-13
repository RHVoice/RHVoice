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

#ifndef RHVOICE_CONFIG_HPP
#define RHVOICE_CONFIG_HPP

#ifndef ANDROID
#include "config.h"
#endif
#include <string>
#include <map>
#include <memory>

#include "str.hpp"
#include "property.hpp"
#include "event_logger.hpp"

namespace RHVoice
{
  class config
  {
  private:
    typedef std::map<std::string,abstract_property*,str::less> registration_map;
    registration_map registered_settings;
    std::shared_ptr<event_logger> logger;

    config(const config&);
    config& operator=(const config&);

  public:

    config():
      logger(new event_logger)
    {
    }

    void register_setting(abstract_property& setting,const std::string& prefix="")
    {
      registered_settings.insert(registration_map::value_type(prefix.empty()?setting.get_name():(prefix+"."+setting.get_name()),&setting));
    }

    bool set(const std::string& name,const std::string& value);
    bool reset(const std::string& name);

    void reset();

    void load(const std::string& file_path);

    void set_logger(const std::shared_ptr<event_logger>& logger_)
    {
      logger=logger_;
    }
  };
}
#endif
