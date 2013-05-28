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

#include "core/path.hpp"
#include "core/config.hpp"
#include "core/engine.hpp"

namespace RHVoice
{
  engine::init_params::init_params():
    data_path(DATA_PATH),
    config_path(CONFIG_PATH)
  {
  }

  engine::engine(const init_params& p):
    data_path(p.data_path),
    config_path(p.config_path),
    version(VERSION),
    languages(path::join(data_path,"languages"),path::join(config_path,"dicts")),
    voices(path::join(data_path,"voices"),languages),
    prefer_primary_language("prefer_primary_language",false)
  {
    if(languages.empty())
      throw no_languages();
    config cfg;
    voice_settings.register_self(cfg);
    text_settings.register_self(cfg);
    verbosity_settings.register_self(cfg);
    cfg.register_setting(prefer_primary_language);
    cfg.register_setting(hts_engine);
    languages.register_settings(cfg);
    voices.register_settings(cfg);
    for(language_list::iterator it(languages.begin());it!=languages.end();++it)
      {
        it->voice_settings.default_to(voice_settings);
        it->text_settings.default_to(text_settings);
      }
    #ifdef WIN32
    cfg.load(path::join(config_path,"RHVoice.ini"));
    #else
    cfg.load(path::join(config_path,"RHVoice.conf"));
    #endif
    if(languages.empty())
      throw no_languages();
  }
}
