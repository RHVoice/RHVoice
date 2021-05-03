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

  const item* item::relative_ptr(const std::string& path) const
  {
    const std::size_t n=path.length();
    if(n==0)
      throw std::invalid_argument("Invalid item path specification");
    std::string name;
    const item* cur_item=this;
    std::size_t i=0;
    std::size_t j=0;
    std::size_t l=0;
    while(i<n && cur_item!=0)
      {
        for(;j<n;++j)
          {
            if(path[j]=='.')
              break;
}
        l=j-i;
        if(l==0)
          throw std::invalid_argument("Invalid item path specification");
        if(l==1)
          {
            if(path[i]=='n')
              cur_item=cur_item->next_ptr();
            else if(path[i]=='p')
              cur_item=cur_item->prev_ptr();
            else
              throw std::invalid_argument("Invalid item path component: "+path);
          }
        else if(l==2)
          {
            if(path[i]=='n'&&path[i+1]=='n')
              cur_item=cur_item->has_next()?cur_item->next().next_ptr():0;
            else if(path[i]=='p'&&path[i+1]=='p')
              cur_item=cur_item->has_prev()?cur_item->prev().prev_ptr():0;
            else
              throw std::invalid_argument("Invalid item path component: "+path);
          }
        else
          {
            if(path[i]=='R'&&path[i+1]==':')
              {
                name.assign(path,i+2,l-2);
                cur_item=cur_item->as_ptr(name);
}
            else if(path.compare(i,l,"parent")==0)
              cur_item=cur_item->parent_ptr();
            else if(path.compare(i,l-1,"daughter")==0)
              {
                if(path[i+l-1]=='1')
                  cur_item=cur_item->first_child_ptr();
                else if(path[i+l-1]=='2')
                  cur_item=cur_item->has_children()?cur_item->first_child().next_ptr():0;
                else if(path[i+l-1]=='n')
                  cur_item=cur_item->last_child_ptr();
                else
                  throw std::invalid_argument("Invalid item path component: "+path);
 }
            else if(path.compare(i,l,"first")==0)
              cur_item=(cur_item->has_parent())?(cur_item->parent().first_child_ptr()):(cur_item->get_relation().first_ptr());
            else if(path.compare(i,l,"last")==0)
              cur_item=(cur_item->has_parent())?(cur_item->parent().last_child_ptr()):(cur_item->get_relation().last_ptr());
            else
              throw std::invalid_argument("Invalid item path component: "+path);
          }
        ++j;
        i=j;
      }
    return cur_item;
  }

  std::pair<std::string,std::string> item::split_feat_spec(const std::string& spec) const
  {
    std::pair<std::string,std::string> res;
    std::string::size_type pos=spec.rfind('.');
    if(pos==std::string::npos)
      res.second=spec;
else
      {
        if(pos==0)
          throw std::invalid_argument("Invalid feature specification");
          res.first.assign(spec,0,pos);
        ++pos;
        if(pos==spec.length())
          throw std::invalid_argument("Invalid feature specification");
        res.second.assign(spec,pos,spec.length()-pos);
}
    return res;
}

  value item::eval(const std::string& feature) const
  {
    std::pair<std::string,std::string> p(split_feat_spec(feature));
    const item& i=p.first.empty()?*this:relative(p.first);
    const value& v=i.get(p.second,true);
    if(!v.empty())
      return v;
    return get_relation().get_utterance().get_language().get_feature_function(p.second).eval(i);
  }

  value item::eval(const std::string& feature,const value& default_value) const
  {
    std::pair<std::string,std::string> p(split_feat_spec(feature));
    const item* i=this;
    if(!p.first.empty())
      {
        i=relative_ptr(p.first);
        if(i==0)
          return default_value;
}
    const value& v=i->get(p.second,true);
    if(!v.empty())
      return v;
    const feature_function* f=get_relation().get_utterance().get_language().get_feature_function_ptr(p.second);
    if(f==0)
      return default_value;
    try
      {
        return f->eval(*i);
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

  void item::clear()
  {
    while(has_children())
      {
        first_child_item->remove();
      }
  }

  void item::remove()
  {
    clear();
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
