/* Copyright (C) 2012  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
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
#include "exception.hpp"
#include "params.hpp"
#include "language.hpp"
#include "voice.hpp"
#include "hts_engine_setting.hpp"

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

    const std::string& get_version() const
    {
      return version;
    }

    static smart_ptr<engine> create(const init_params& p=init_params())
    {
      return smart_ptr<engine>(new engine(p));
    }
      
  private:
    engine(const engine&);
    engine& operator=(const engine&);

    std::string data_path,config_path,version;
    language_list languages;
    voice_list voices;

  public:
    voice_params voice_settings;
    text_params text_settings;
    verbosity_params verbosity_settings;
    bool_property prefer_primary_language;
    hts_engine_setting hts_engine;
  };
}
#endif
