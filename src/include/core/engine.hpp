/* Copyright (C) 2012, 2014, 2017, 2019  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 2.1 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef RHVOICE_ENGINE_HPP
#define RHVOICE_ENGINE_HPP

#include <string>
#include <map>
#include <set>
#include "exception.hpp"
#include "params.hpp"
#include "language.hpp"
#include "voice.hpp"
#include "voice_profile.hpp"
#include "quality_setting.hpp"
#include "event_logger.hpp"

namespace RHVoice
{
  class no_languages: public exception
  {
  public:
    no_languages():
      exception("No language resources are available")
    {
    }
  };

  class engine
  {
  public:
    struct init_params
    {
      init_params();
      std::string data_path,config_path;
      std::vector<std::string> resource_paths;
      smart_ptr<event_logger> logger;

      std::vector<std::string> get_language_paths() const
      {
        return get_resource_paths("language");
      }

      std::vector<std::string> get_voice_paths() const
      {
        return get_resource_paths("voice");
      }

    private:
      std::vector<std::string> get_resource_paths(const std::string& type) const;
    };

    explicit engine(const init_params& p=init_params());

    const std::string& get_data_path() const
    {
      return data_path;
    }

    const std::string& get_config_path() const
    {
      return config_path;
    }

    language_list& get_languages()
    {
      return languages;
    }

    const language_list& get_languages() const
    {
      return languages;
    }

    voice_list& get_voices()
    {
      return voices;
    }

    const voice_list& get_voices() const
    {
      return voices;
    }

    const std::set<voice_profile>& get_voice_profiles() const
    {
      return voice_profiles;
    }

    const std::string& get_version() const
    {
      return version;
    }

    static smart_ptr<engine> create(const init_params& p=init_params())
    {
      return smart_ptr<engine>(new engine(p));
    }
      
    voice_profile create_voice_profile(const std::string& spec) const;

    const smart_ptr<event_logger>& get_logger() const
    {
      return logger;
    }


    bool configure(const std::string& key,const std::string& value)
    {
      return cfg.set(key,value);
    }

    voice_profile get_fallback_voice_profile() const;

  private:
    engine(const engine&);
    engine& operator=(const engine&);

    string_property voice_profiles_spec;
    std::string data_path,config_path,version;
    language_list languages;
    voice_list voices;
    std::set<voice_profile> voice_profiles;
    smart_ptr<event_logger> logger;
    config cfg;

    void create_voice_profiles();

  public:
    voice_params voice_settings;
    text_params text_settings;
    verbosity_params verbosity_settings;
    bool_property prefer_primary_language;
    quality_setting quality;
  };
}
#endif
