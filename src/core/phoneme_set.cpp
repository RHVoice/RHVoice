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

#include <vector>
#include <fstream>
#include <iterator>
#include <algorithm>
#include "rapidxml/rapidxml_iterators.hpp"
#include "core/io.hpp"
#include "core/phoneme_set.hpp"

namespace RHVoice
{
  phoneme_set::phoneme_set(const std::string& file_path)
  {
    std::ifstream infile;
    io::open_ifstream(infile,file_path);
    std::vector<char> text;
    std::copy(std::istreambuf_iterator<char>(infile),std::istreambuf_iterator<char>(),std::back_inserter(text));
    text.push_back('\0');
    rapidxml::xml_document<> doc;
    doc.parse<0>(&text[0]);
    rapidxml::xml_node<>* root_node=doc.first_node("phonemes");
    if ((root_node == 0) || (root_node->type() != rapidxml::node_element))
      {
        std::string err_msg("Phoneme set format is incorrect");
        throw file_format_error(err_msg);
      }
    rapidxml::node_iterator<char> last_node;
    rapidxml::attribute_iterator<char> last_attr;
    std::string attr_name;
    for(rapidxml::node_iterator<char> node_iter(root_node);node_iter!=last_node;++node_iter)
      {
        if(node_iter->type()!=rapidxml::node_element)
          continue;
        for(rapidxml::attribute_iterator<char> attr_iter(&(*node_iter));attr_iter!=last_attr;++attr_iter)
          {
            attr_name.assign(attr_iter->name(),attr_iter->name_size());
            if(attr_name!="name")
              feature_set.insert(attr_name);
          }
      }
    for(rapidxml::node_iterator<char> node_iter(root_node);node_iter!=last_node;++node_iter)
      {
        if(node_iter->type()!=rapidxml::node_element)
          continue;
        phoneme ph(&(*node_iter),feature_set);
        phonemes.insert(phoneme_map::value_type(ph.get_name(),ph));
      }
  }

  phoneme::phoneme(rapidxml::xml_node<>* element,const phoneme_feature_set& feature_set)
  {
    for(phoneme_feature_set::const_iterator iter(feature_set.begin());iter!=feature_set.end();++iter)
      {
        features.insert(phoneme_feature_map::value_type(*iter,"0"));
      }
    rapidxml::attribute_iterator<char> last_attr;
    std::string attr_name;
    for(rapidxml::attribute_iterator<char> attr_iter(element);attr_iter!=last_attr;++attr_iter)
      {
        attr_name.assign(attr_iter->name(),attr_iter->name_size());
        if(attr_name=="name")
          name=attr_iter->value();
        else
          features[attr_name]=attr_iter->value();
      }
  }
}
