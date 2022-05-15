/* Copyright (C) 2012, 2018, 2019  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_VOICE_HPP
#define RHVOICE_VOICE_HPP

#include <string>
#include <functional>
#include <set>
#include "RHVoice_common.h"

#include "str.hpp"
#include "property.hpp"
#include "resource.hpp"
#include "params.hpp"
#include "language.hpp"
#include "sample_rate.hpp"
#include "hts_engine_pool.hpp"
#include "hts_engine_call.hpp"

namespace RHVoice
{
  class utterance;
  class client;
  class voice_info;

class voice
{
public:
  explicit voice(const voice_info& info);

  const voice_info& get_info() const
  {
    return info;
  }

  bool synthesize(const utterance& u,client& c) const;

private:
  voice(const voice&);
  voice& operator=(const voice&);

  const voice_info& info;
  mutable hts_engine_pool engine_pool;
};

  class voice_info: public resource_info<voice>
  {
  public:
    voice_info(unsigned int fmt,const std::string& data_path,language_list& languages);

    language_list::const_iterator get_language() const
    {
      return voice_language;
    }

    const std::string& get_alpha2_country_code() const
    {
      return alpha2_country_code;
    }

    const std::string& get_alpha3_country_code() const
    {
      return alpha3_country_code;
    }

    sample_rate_t get_sample_rate() const
    {
      return sample_rate;
    }

    RHVoice_voice_gender get_gender() const
    {
      return gender;
    }

    std::string get_country() const
    {
      if(country.is_set())
        return country;
      else
        return voice_language->get_country();
    }

    unsigned int get_format() const
    {
      return format;
    }

    const std::string& get_id() const
    {
      return id;
    }

    voice_params settings;

    void register_settings(config& cfg);

    virtual bool is_enabled() const
    {
      return enabled&&voice_language->is_enabled();
    }

    bool is_preferred() const
    {
      return preferred;
    }

    bool supports_utt_type(const std::string& ut) const
    {
      if(ut=="s")
        return true;
      return extra_utt_types.includes(ut);
}

    const stream_params& get_stream_settings() const
    {
      return *stream_settings;
    }

    void set_stream_settings(const stream_params& s)
    {
      stream_settings=&s;
    }

  private:
    std::shared_ptr<voice> create_instance() const
    {
      return std::shared_ptr<voice>(new voice(*this));
    }

    const unsigned int format;
    language_list::const_iterator voice_language;
    std::string alpha2_country_code;
    std::string alpha3_country_code;
    sample_rate_property sample_rate;
    enum_property<RHVoice_voice_gender> gender;
    std::string id;
    bool_property enabled,preferred;
    string_property country;
    stringset_property extra_utt_types;
    const stream_params* stream_settings{nullptr};
  };

  class voice_list: public resource_list<voice_info>
  {
  public:
    voice_list(const std::vector<std::string>& voice_paths,language_list& languages,const event_logger& logger);
  };

  class voice_search_criteria: public std::unary_function<const voice_info&,bool>
  {
  public:
    voice_search_criteria():
      voice_language(0),
      preferred(false)
    {
    }

    void add_name(const std::string& name)
    {
      names.insert(name);
    }

    template<typename input_iterator>
    void add_names(input_iterator start,input_iterator end)
    {
      names.insert(start,end);
    }

    void clear_names()
    {
      names.clear();
    }

    void set_language(const language_info& lang)
    {
      voice_language=&lang;
    }

    void clear_language()
    {
      voice_language=0;
    }

    void set_preferred()
    {
      preferred=true;
    }

    void clear_preferred()
    {
      preferred=false;
    }

    bool operator()(const voice_info& info) const;

    bool empty() const
    {
      return (names.empty()&&
              (voice_language==0)&&
              (!preferred));
    }

  private:
    std::set<std::string,str::less> names;
    const language_info* voice_language;
    bool preferred;
  };
}
#endif
