/* Copyright (C) 2012  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_PHONEME_SET_HPP
#define RHVOICE_PHONEME_SET_HPP

#include <stdexcept>
#include <string>
#include <set>
#include <map>
#include "rapidxml/rapidxml.hpp"

namespace RHVoice
{

  typedef std::set<std::string> phoneme_feature_set;

  class phoneme
  {
  private:
    typedef std::map<std::string,std::string> phoneme_feature_map;

    std::string name;
    phoneme_feature_map features;

  public:
    phoneme(rapidxml::xml_node<>* element,const phoneme_feature_set& feature_set);

    const std::string& get_name() const
    {
      return name;
    }

    const std::string& operator[](const std::string& name) const
    {
      phoneme_feature_map::const_iterator it=features.find(name);
      if(it==features.end())
        throw std::invalid_argument("Unknown phoneme feature");
      else
        return (it->second);
    }
  };

  class phoneme_set
  {
  private:
    typedef std::map<std::string,phoneme> phoneme_map;

    phoneme_map phonemes;
    phoneme_feature_set feature_set;

  public:
    explicit phoneme_set(const std::string& file_path);

    const phoneme& get_phoneme(const std::string& name) const
    {
      phoneme_map::const_iterator it=phonemes.find(name);
      if(it==phonemes.end())
        throw std::invalid_argument("Unknown phoneme: "+name);
      else
        return (it->second);
    }

    const phoneme_feature_set& get_feature_set() const
    {
      return feature_set;
    }
  };
}
#endif
