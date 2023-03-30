/* Copyright (C) 2012, 2014, 2019, 2021  Olga Yakovleva <olga@rhvoice.org> */

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

#include "core/path.hpp"
#include "core/config.hpp"
#include "core/engine.hpp"
#ifndef ANDROID
#include "config.h"
#endif

#include <cstdlib>

namespace
{
  const std::string tag("engine");
}

namespace RHVoice
{
  engine::init_params::init_params():
    data_path(DATA_PATH),
    config_path(CONFIG_PATH),
    logger(new event_logger)
  {
    char* data_path_env = getenv("RHVOICE_DATA_PATH");
    if (data_path_env) data_path = data_path_env;
    char* config_path_env = getenv("RHVOICE_CONFIG_PATH");
    if (config_path_env) config_path = config_path_env;
  }

  std::vector<std::string> engine::init_params::get_resource_paths(const std::string& type) const
  {
    std::vector<std::string> result;
    if(!data_path.empty()&&path::isdir(data_path))
      {
        std::string parent_path(path::join(data_path,type+"s"));
        if(path::isdir(parent_path))
          {
            for(path::directory dir(parent_path);!dir.done();dir.next())
              {
                std::string resource_path(path::join(parent_path,dir.get()));
                if(path::isdir(resource_path))
                  result.push_back(resource_path);
              }
          }
      }
    for(std::vector<std::string>::const_iterator it=resource_paths.begin();it!=resource_paths.end();++it)
      {
        if(path::isdir(*it)&&path::isfile(path::join(*it,type+".info")))
          result.push_back(*it);
      }
    return result;
  }

  engine::engine(const init_params& p):
    voice_profiles_spec("voice_profiles"),
    data_path(p.data_path),
    config_path(p.config_path),
    version(VERSION),
    languages(p.get_language_paths(),path::join(config_path,"dicts"),*p.logger),
    voices(p.get_voice_paths(),languages,*p.logger),
    logger(p.logger),
    prefer_primary_language("prefer_primary_language",true),
    enable_bilingual("enable_bilingual", true)
  {
    logger->log(tag,RHVoice_log_level_info,"creating a new engine");
    if(p.has_data_paths() && languages.empty())
      throw no_languages();
    cfg.set_logger(logger);
    cfg.register_setting(voice_profiles_spec);
    voice_settings.register_self(cfg);
    text_settings.register_self(cfg);
    verbosity_settings.register_self(cfg);
    cfg.register_setting(prefer_primary_language);
    cfg.register_setting(enable_bilingual);
    cfg.register_setting(quality);
    stream_settings.register_self(cfg);
    languages.register_settings(cfg);
    voices.register_settings(cfg);
    for(language_list::iterator it(languages.begin());it!=languages.end();++it)
      {
        it->voice_settings.default_to(voice_settings);
        it->text_settings.default_to(text_settings);
      }
    for(auto& v: voices)
      v.set_stream_settings(stream_settings);
    #ifdef WIN32
    cfg.load(path::join(config_path,"RHVoice.ini"));
    #else
    cfg.load(path::join(config_path,"RHVoice.conf"));
    #endif
    if(p.has_data_paths() && languages.empty())
      throw no_languages();
    create_voice_profiles();
    #if ENABLE_PKG
    if(!p.pkg_path.empty())
      {
        logger->log(tag,RHVoice_log_level_info,"getting the package client instance");
        pkgc=pkg::package_client::get(p.pkg_path);
      }
    #endif
    logger->log(tag,RHVoice_log_level_info,"engine created");
  }

  voice_profile engine::create_voice_profile(const std::string& spec) const
  {
    voice_profile profile;
    std::string name;
    voice_list::const_iterator v;
    str::tokenizer<str::is_equal_to> tok(spec,str::is_equal_to('+'));
    for(str::tokenizer<str::is_equal_to>::iterator it=tok.begin();it!=tok.end();++it)
      {
        name=str::trim(*it);
        v=voices.find(name);
        if(v!=voices.end())
          profile.add(v);
      }
    return profile;
  }

  void engine::create_voice_profiles()
  {
    for(voice_list::const_iterator it=voices.begin();it!=voices.end();++it)
      {
        voice_profiles.insert(voice_profile(it));
      }
    if(!voice_profiles_spec.is_set())
      return;
    std::string list_spec_string=voice_profiles_spec.get();
    str::tokenizer<str::is_equal_to> tok(list_spec_string,str::is_equal_to(','));
    for(str::tokenizer<str::is_equal_to>::iterator it=tok.begin();it!=tok.end();++it)
      {
        std::string profile_spec_string=str::trim(*it);
        voice_profile profile=create_voice_profile(profile_spec_string);
        if(!profile.empty())
          voice_profiles.insert(profile);
      }
  }

  voice_profile engine::get_fallback_voice_profile() const
  {
    voice_profile p;
    if(voices.empty())
      return p;
    for(voice_list::const_iterator it=voices.begin();it!=voices.end();++it)
      {
        if(it->get_language()->get_alpha3_code()=="eng")
          {
            p.add(it);
            break;
}
}
    for(voice_list::const_iterator it=voices.begin();it!=voices.end();++it)
      {
        if(it->get_language()->get_alpha3_code()=="rus")
          {
            p.add(it);
            break;
}
}
    if(p.empty())
      p.add(voices.begin());
    return p;
}
}
