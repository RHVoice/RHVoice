/* Copyright (C) 2013, 2017  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_VOICE_PROFILE_HPP
#define RHVOICE_VOICE_PROFILE_HPP

#include <string>
#include <vector>
#include "voice.hpp"

namespace RHVoice
{
  class voice_profile
  {
  private:
    typedef std::vector<voice_list::const_iterator> list;

    list voices;
    std::string name;

  public:
    typedef std::vector<voice_list::const_iterator>::const_iterator iterator;

    voice_profile() = default;

    explicit voice_profile(voice_list::const_iterator only_voice)
    {
      voices.push_back(only_voice);
      name=only_voice->get_name();
    }

    bool operator<(const voice_profile& other) const
    {
      return (name<other.name);
    }

    const std::string& get_name() const
    {
      return name;
    }

    iterator begin() const
    {
      return voices.begin();
    }

    iterator end() const
    {
      return voices.end();
    }

    bool add(voice_list::const_iterator v)
    {
      language_list::const_iterator lang=v->get_language();
      for(iterator it=begin();it!=end();++it)
        {
          if((*it)->get_language()==lang)
            return false;
        }
      voices.push_back(v);
      if(name.empty())
        name=v->get_name();
      else
        {
          name+="+";
          name+=v->get_name();
        }
      return true;
    }

    bool empty() const
    {
      return voices.empty();
    }

    std::size_t voice_count() const
    {
      return voices.size();
    }

    voice_list::const_iterator primary() const
    {
      return voices.at(0);
    }

    voice_list::const_iterator get_voice(std::size_t i) const
    {
      return voices.at(i);
    }


    iterator voice_for_language(language_list::const_iterator lang) const
    {
      for(iterator it=begin();it!=end();++it)
        {
          voice_list::const_iterator v=*it;
          if(v->get_language()==lang)
            return it;
        }
      return end();
    }

    template<typename text_iterator> iterator voice_for_text(text_iterator text_start,text_iterator text_end) const;
  };

  template<typename text_iterator>
  voice_profile::iterator voice_profile::voice_for_text(text_iterator text_start,text_iterator text_end) const
  {
    iterator best=end();
    voice_list::const_iterator v;
    language_list::const_iterator lang;
    std::size_t max_count=0;
    std::size_t count;
    for(iterator it=begin();it!=end();++it)
      {
        v=*it;
        lang=v->get_language();
        if((best!=end())&&lang->has_common_letters(*((*best)->get_language())))
          continue;
        count=lang->count_letters_in_text(text_start,text_end);
        if(count>max_count)
          {
            best=it;
            max_count=count;
            if(it==begin())
              return best;
          }
      }
    return best;
  }
}
#endif
