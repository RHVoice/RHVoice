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
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <utility>
#include <functional>
#include <algorithm>
#include <vector>
#include <stdexcept>
#include "core/item.hpp"
#include "core/relation.hpp"
#include "core/utterance.hpp"
#include "core/language.hpp"
#include "core/str.hpp"

namespace RHVoice
{
  const value item::empty_value;

  void item::init()
    {
      std::pair<self_ref_map::iterator,bool> res=data->self_refs.insert(self_ref_map::value_type(relation_ptr->get_name(),this));
      if(!res.second)
        throw duplicate_item();
  }

  value item::eval(const std::string& feature) const
  {
    std::string name;
    const std::size_t n=feature.length();
    if(n==0)
      throw std::invalid_argument("Invalid feature specification");
    std::size_t i=0;
    std::size_t l=0;
    const item* cur_item=this;
    for(std::size_t j=0;j<n;++j)
      {
        if(feature[j]!='.')
          {
            ++l;
            continue;
}
        if(l==0)
          throw std::invalid_argument("Invalid feature specification");
        if(l==1)
          {
            if(feature[i]=='n')
              cur_item=&(cur_item->next());
            else if(feature[i]=='p')
              cur_item=&(cur_item->prev());
            else
              throw std::invalid_argument("Invalid item path component");
          }
        else if(l==2)
          {
            if(feature[i]=='n'&&feature[i+1]=='n')
              cur_item=&(cur_item->next().next());
            else if(feature[i]=='p'&&feature[i+1]=='p')
              cur_item=&(cur_item->prev().prev());
            else
              throw std::invalid_argument("Invalid item path component");
          }
        else
          {
            if(feature[i]=='R'&&feature[i+1]==':')
              {
                name.assign(feature,i+2,l-2);
                cur_item=&(cur_item->as(name));
}
            else if(feature.compare(i,l,"parent")==0)
              cur_item=&(cur_item->parent());
            else if(feature.compare(i,l-1,"daughter")==0)
              {
                if(feature[i+l-1]=='1')
                  cur_item=&(cur_item->first_child());
                else if(feature[i+l-1]=='2')
                  cur_item=&(cur_item->first_child().next());
                else if(feature[i+l-1]=='n')
                  cur_item=&(cur_item->last_child());
                else
                  throw std::invalid_argument("Invalid item path component");
 }
            else if(feature.compare(i,l,"first")==0)
              cur_item=&((cur_item->has_parent())?(cur_item->parent().first_child()):(cur_item->get_relation().first()));
            else if(feature.compare(i,l,"last")==0)
              cur_item=&((cur_item->has_parent())?(cur_item->parent().last_child()):(cur_item->get_relation().last()));
            else
              throw std::invalid_argument("Invalid item path component");
          }
        i=j+1;
        l=0;
      }
    if(l==0)
      throw std::invalid_argument("Invalid feature specification");
    name.assign(feature,i,l);
    const value& val=cur_item->get(name,true);
    if(!val.empty())
      return val;
    return cur_item->get_relation().get_utterance().get_language().get_feature_function(name).eval(*cur_item);
  }

  value item::eval(const std::string& feature,const value& default_value) const
  {
    try
      {
        return eval(feature);
      }
    catch(const lookup_error&)
      {
        return default_value;
      }
  }

  item* item::append_item(item* other)
  {
    if(next_item)
      {
        next_item->prev_item=other;
        other->next_item=next_item;
      }
    else
      {
        if(parent_item)
          parent_item->last_child_item=other;
        else
          relation_ptr->tail=other;
      }
    next_item=other;
    other->prev_item=this;
    return other;
  }

  item* item::prepend_item(item* other)
  {
    if(prev_item)
      {
        prev_item->next_item=other;
        other->prev_item=prev_item;
      }
    else
      {
        if(parent_item)
          parent_item->first_child_item=other;
        else
          relation_ptr->head=other;
      }
    prev_item=other;
    other->next_item=this;
    return other;
  }

  item& item::append(item& other)
  {
    return *append_item(parent_item?(new item(other,parent_item)):(new item(other,relation_ptr)));
  }

  item& item::append()
  {
    return *append_item(parent_item?(new item(parent_item)):(new item(relation_ptr)));
  }

  item& item::prepend(item& other)
  {
    return *prepend_item(parent_item?(new item(other,parent_item)):(new item(other,relation_ptr)));
  }

  item& item::prepend()
  {
    return *prepend_item(parent_item?(new item(parent_item)):(new item(relation_ptr)));
  }

  item& item::append_child(item& other)
  {
    item* new_item=new item(other,this);
    if(last_child_item)
      return *(last_child_item->append_item(new_item));
    else
      {
        first_child_item=last_child_item=new_item;
        return *last_child_item;
      }
  }

  item& item::append_child()
  {
    item* new_item=new item(this);
    if(last_child_item)
      return *(last_child_item->append_item(new_item));
    else
      {
        first_child_item=last_child_item=new_item;
        return *last_child_item;
      }
  }

  item& item::prepend_child(item& other)
  {
    item* new_item=new item(other,this);
    if(first_child_item)
      return *(first_child_item->prepend_item(new_item));
    else
      {
        first_child_item=last_child_item=new_item;
        return *first_child_item;
      }
  }

  item& item::prepend_child()
  {
    item* new_item=new item(this);
    if(first_child_item)
      return *(first_child_item->prepend_item(new_item));
    else
      {
        first_child_item=last_child_item=new_item;
        return *first_child_item;
      }
  }

  void item::remove()
  {
    while(has_children())
      {
        first_child_item->remove();
      }
    data->self_refs.erase(relation_ptr->get_name());
    if(prev_item)
      prev_item->next_item=next_item;
    else
      {
        if(parent_item)
          parent_item->first_child_item=next_item;
        else
          relation_ptr->head=next_item;
      }
    if(next_item)
      next_item->prev_item=prev_item;
    else
      {
        if(parent_item)
          parent_item->last_child_item=prev_item;
        else
          relation_ptr->tail=prev_item;
      }
    delete this;
  }

  item::iterator item::iterator::operator++(int)
    {
      iterator tmp(*this);
      ++(*this);
      return tmp;
    }

  item::iterator& item::iterator::operator--()
  {
    c=c?(c->prev_item):(p?(p->last_child_item):(r->tail));
    return *this;
  }

  item::iterator item::iterator::operator--(int)
  {
    iterator tmp(*this);
    --(*this);
    return tmp;
  }

  item::const_iterator item::const_iterator::operator++(int)
  {
    const_iterator tmp(*this);
    ++(*this);
    return tmp;
  }

  item::const_iterator& item::const_iterator::operator--()
  {
    c=c?(c->prev_item):(p?(p->last_child_item):(r->tail));
    return *this;
  }

  item::const_iterator item::const_iterator::operator--(int)
  {
    const_iterator tmp(*this);
    --(*this);
    return tmp;
  }
}
