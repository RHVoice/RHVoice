/* Copyright (C) 2012, 2018  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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


#include <locale>
#include "boost/algorithm/string.hpp"
#include "core/path.hpp"
#include "core/exception.hpp"
#include "core/config.hpp"
#include "core/voice.hpp"
#include "core/event_logger.hpp"

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

  voice_info::voice_info(unsigned int fmt,const std::string& data_path,language_list& languages):
    format(fmt),
    gender("gender",RHVoice_voice_gender_unknown),
    enabled("enabled",true),
    preferred("preferred",false),
    country("country"),
    extra_utt_types("extra_utt_types")
  {
    gender.define("male",RHVoice_voice_gender_male);
    gender.define("female",RHVoice_voice_gender_female);
    set_data_path(data_path);
    string_property name("name");
    string_property android_id("android_id");
    string_property voice_id("id");
    voice_id.default_to(android_id);
    enum_string_property language_name("language");
    for(language_list::const_iterator it(languages.begin());it!=languages.end();++it)
      {
        language_name.define(it->get_name());
      }
    config cfg;
    cfg.register_setting(name);
    cfg.register_setting(android_id);
    cfg.register_setting(voice_id);
    cfg.register_setting(language_name);
    cfg.register_setting(country);
    cfg.register_setting(sample_rate);
    cfg.register_setting(gender);
    cfg.register_setting(extra_utt_types);
    cfg.load(path::join(data_path,"voice.info"));
    if(!name.is_set())
      throw file_format_error("Voice name is not set");
    set_name(name);
    if(voice_id.is_set(true))
      id=voice_id;
    else {
      id=boost::replace_all_copy(boost::to_lower_copy(name.get(), std::locale::classic()), "-", "_");
    }
    if(!language_name.is_set())
      throw file_format_error("Voice language is not set");
    language_list::iterator lang=languages.find(language_name);
    voice_language=lang;
    settings.default_to(voice_language->voice_settings);
    std::string c(get_country());
    if(c.size()>=3)
      {
        alpha3_country_code=c.substr(c.size()-3,3);
        if(c.size()==6)
          alpha2_country_code=c.substr(0,2);
}
  }

  void voice_info::register_settings(config& cfg)
  {
    std::string prefix("voices."+get_name());
    settings.register_self(cfg,prefix);
    cfg.register_setting(enabled,prefix);
    cfg.register_setting(preferred,prefix);
  }

  voice_list::voice_list(const std::vector<std::string>& voice_paths,language_list& languages,const event_logger& logger)
  {
    std::string tag("voice_list");
    for(std::vector<std::string>::const_iterator it=voice_paths.begin();it!=voice_paths.end();++it)
      {
        if(path::isdir(*it))
          {
            logger.log(tag,RHVoice_log_level_info,std::string("Path: ")+(*it));
            std::shared_ptr<voice_info> v;
            version_info ver;
            try
              {
                resource_description desc("voice",*it);
                logger.log(tag,RHVoice_log_level_info,std::string("Voice resource: ")+desc.name.get()+std::string(", format: ")+str::to_string(desc.format.get())+std::string(", revision: ")+str::to_string(desc.revision.get()));
                ver=version_info(desc.format,desc.revision);
                if(desc.format>=3&&can_add(desc.name,ver))
                  v.reset(new voice_info(desc.format,*it,languages));
                else
                  logger.log(tag,RHVoice_log_level_error,"Unsupported voice format");
              }
            catch(...)
              {
                logger.log(tag,RHVoice_log_level_error,"Voice info creation failed");
              }
            if(v)
              add(v,ver);
          }
      }
  }

  bool voice_search_criteria::operator()(const voice_info& info) const
  {
    const language_info& lang=*(info.get_language());
    if((voice_language==0)||(voice_language==&lang))
      if(names.empty()||(names.find(info.get_name())!=names.end()))
        if(!preferred||info.is_preferred())
          return true;
    return false;
  }
}
