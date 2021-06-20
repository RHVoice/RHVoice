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

#include <utility>
#include "core/utterance.hpp"
#include "core/relation.hpp"

namespace RHVoice
{
  relation& utterance::add_relation(const std::string& name)
  {
    std::shared_ptr<relation> rel(new relation(name,*this));
    std::pair<relation_map::iterator,bool> res=relations.insert(relation_map::value_type(name,rel));
    if(!res.second)
      throw relation_exists();
    return *(res.first->second);
    }

  void utterance::remove_relation(const std::string& name)
  {
    relation_map::iterator it(relations.find(name));
    if(it==relations.end())
      throw relation_not_found(name);
    relations.erase(it);
  }
}
