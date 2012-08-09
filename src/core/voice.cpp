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
#include "core/exception.hpp"
#include "core/config.hpp"
#include "core/voice.hpp"

namespace RHVoice
{
  voice::voice(const voice_info& info_):
    info(info_),
    engine_pool(info_)
  {
  }

  bool voice::synthesize(const utterance& u,client& c) const
  {
    hts_engine_call call(engine_pool,u,c);
    return call.execute();
  }

  voice_info::voice_info(const std::string& data_path,language_list& languages):
    sample_rate("sample_rate",sample_rate_16k),
    enabled("enabled",true),
    preferred("preferred",false)
  {
    sample_rate.define("16k",sample_rate_16k);
    sample_rate.define("44k",sample_rate_44k);
    sample_rate.define("48k",sample_rate_48k);
    set_data_path(data_path);
    string_property name("name");
    enum_string_property language_name("language");
    for(language_list::const_iterator it(languages.begin());it!=languages.end();++it)
      {
        language_name.define(it->get_name());
      }
    config cfg(path::join(data_path,"voice.info"));
    cfg.register_setting(name);
    cfg.register_setting(language_name);
    cfg.register_setting(sample_rate);
    cfg.load();
    if(!name.is_set())
      throw file_format_error("Voice name is not set");
    set_name(name);
    if(!language_name.is_set())
      throw file_format_error("Voice language is not set");
    language_list::iterator lang=languages.find(language_name);
    voice_language=lang;
    settings.default_to(voice_language->voice_settings);
  }

  void voice_info::register_settings(config& cfg)
  {
    std::string prefix("voices."+get_name());
    settings.register_self(cfg,prefix);
    cfg.register_setting(enabled,prefix);
    cfg.register_setting(preferred,prefix);
  }

  voice_list::voice_list(const std::string& data_path,language_list& languages)
  {
    for(path::directory dir(data_path);!dir.done();dir.next())
      {
        std::string voice_path(path::join(data_path,dir.get()));
        if(path::isdir(voice_path))
          {
            smart_ptr<voice_info> v;
            try
              {
                v.reset(new voice_info(voice_path,languages));
              }
            catch(...)
              {
              }
            if(!v.empty())
              add(v);
          }
      }
  }

  bool voice_search_criteria::operator()(const voice_info& info) const
  {
    if((voice_language==language_list::const_iterator())||(voice_language==info.get_language()))
      if(names.empty()||(names.find(info.get_name())!=names.end()))
        if(!preferred||info.is_preferred())
          return true;
    return false;
  }
}
