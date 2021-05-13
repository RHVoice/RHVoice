/* Copyright (C) 2013  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_TRIE_HPP
#define RHVOICE_TRIE_HPP

#include <vector>
#include <stack>
#include <algorithm>
#include <utility>
#include <memory>
#include <stdexcept>
#include "str.hpp"

namespace RHVoice
{
  template<typename E,typename T,class L=str::to_lower>
  class trie
  {
  public:
    typedef E element_type;
    typedef T value_type;

  private:
    typedef std::vector<element_type> label_type;
    typedef typename label_type::const_iterator label_iterator;

    trie(const trie&);
    trie& operator=(const trie&);

    class node
    {
    public:
      typedef node* pointer;
      typedef std::vector<pointer> child_list;
      typedef typename child_list::iterator child_iterator;

      label_type label;
      child_list children;
      value_type* value;

      const element_type& get_first_element() const
      {
        return label[0];
      }

    private:
      node(const node&);
      node& operator=(const node&);

      struct compare
      {
        bool operator()(const pointer& p1,const pointer& p2) const
        {
          return (p1->get_first_element()<p2->get_first_element());
        }

        bool operator()(const pointer& p,const element_type& e) const
        {
          return (p->get_first_element()<e);
        }

        bool operator()(const element_type& e,const pointer& p) const
        {
          return (e<p->get_first_element());
        }
      };

    public:
      node():
        value(0)
      {
      }

      template<typename forward_iterator>
      node(forward_iterator first,forward_iterator last):
        label(first,last),
        value(0)
      {
      }

      ~node();

      void sort();

      pointer find_child_l(const element_type& e)
      {
      for(child_iterator it=children.begin();it!=children.end();++it)
        {
      if((*it)->get_first_element()==e)
        return *it;
    }
      return 0;
    }

      pointer find_child_b(const element_type& e)
      {
        child_iterator it=std::lower_bound(children.begin(),children.end(),e,compare());
        return ((it==children.end())?0:(((*it)->get_first_element()==e)?(*it):0));
      }

      template<typename forward_iterator>
      std::pair<label_iterator,forward_iterator> get_common_prefix(forward_iterator first,forward_iterator last) const
      {
        std::pair<label_iterator,forward_iterator> result(label.begin(),first);
        for(;(result.first!=label.end())&&(result.second!=last)&&(*result.first==*result.second);++result.first,++result.second);
        return result;
      }

      template<typename forward_iterator,class to_lower>
      std::pair<label_iterator,forward_iterator> get_common_prefix(forward_iterator first,forward_iterator last,const to_lower& l) const
      {
        std::pair<label_iterator,forward_iterator> result(label.begin(),first);
        for(;(result.first!=label.end())&&(result.second!=last)&&((*result.first==*result.second)||(*result.first==l(*result.second)));++result.first,++result.second);
        return result;
      }

      template<typename forward_iterator>
      pointer add_child(forward_iterator first,forward_iterator last)
      {
        std::unique_ptr<node> child(new node(first,last));
        children.push_back(child.get());
        return child.release();
      }

      void split(label_iterator pos)
      {
        label_type new_label(label_iterator(label.begin()),pos);
        std::unique_ptr<node> child(new node(pos,label_iterator(label.end())));
        child->children.reserve(1);
        child->value=value;
        value=0;
        label.swap(new_label);
        children.swap(child->children);
        children.push_back(child.get());
        child.release();
      }
    };

    node* root;
    bool sorted;
    L tolower;

    template<typename forward_iterator>
    node* get_node(forward_iterator first,forward_iterator last);

  public:
    explicit trie(const L& l=L()):
      root(new node),
      sorted(false),
      tolower(l)
    {
    }

    ~trie()
    {
      delete root;
    }

    void sort()
    {
      root->sort();
      sorted=true;
    }

    template<typename forward_iterator>
    value_type& insert(forward_iterator first,forward_iterator last)
    {
      std::unique_ptr<value_type> val(new value_type());
      node* child=get_node(first,last);
      if(child->value==0)
        child->value=val.release();
      return *(child->value);
    }

    template<typename forward_iterator>
    value_type& insert(forward_iterator first,forward_iterator last,const value_type& value)
    {
      std::unique_ptr<value_type> val(new value_type(value));
      node* child=get_node(first,last);
      delete child->value;
      child->value=val.release();
      return *(child->value);
    }

    template<typename forward_iterator>
    std::pair<forward_iterator,value_type*> find(forward_iterator first,forward_iterator last) const;
    template<typename forward_iterator>
    std::pair<forward_iterator,value_type*> search(forward_iterator first,forward_iterator last) const;
  };

  template<typename E,typename T,class L>
  trie<E,T,L>::node::~node()
  {
    for(child_iterator it=children.begin();it!=children.end();++it)
      {
        delete *it;
      }
    delete value;
  }

  template<typename E,typename T,class L>
  void trie<E,T,L>::node::sort()
  {
    if(!children.empty())
      {
        for(child_iterator it=children.begin();it!=children.end();++it)
          {
            (*it)->sort();
          }
        std::sort(children.begin(),children.end(),compare());
      }
  }

  template<typename E,typename T,class L> template<typename forward_iterator>
  std::pair<forward_iterator,T*> trie<E,T,L>::find(forward_iterator first,forward_iterator last) const
  {
    if(!sorted)
      throw std::logic_error("The trie must be sorted before searching");
    std::pair<forward_iterator,value_type*> result(first,0);
    node* current=root;
    std::pair<label_iterator,forward_iterator> positions(current->label.begin(),first);
    while(current)
      {
        positions=current->get_common_prefix(positions.second,last);
        if(positions.first==current->label.end())
          {
            if(current->value)
              {
                result.first=positions.second;
                result.second=current->value;
              }
          }
        else
          break;
        if(positions.second==last)
          break;
        current=current->find_child_b(*positions.second);
      }
    return result;
  }

  template<typename E,typename T,class L> template<typename forward_iterator>
  std::pair<forward_iterator,T*> trie<E,T,L>::search(forward_iterator first,forward_iterator last) const
  {
    if(!sorted)
      throw std::logic_error("The trie must be sorted before searching");
    std::stack<std::pair<forward_iterator,node*> > alternatives;
    std::pair<forward_iterator,value_type*> result;
    result.first=first;
    result.second=0;
    node* current=root;
    std::pair<label_iterator,forward_iterator> positions(current->label.begin(),first);
    node* alternative=0;
    element_type lower;
    while(current)
      {
        positions=current->get_common_prefix(positions.second,last,tolower);
        if(positions.first==current->label.end())
          {
            if(current->value)
              {
                result.first=positions.second;
                result.second=current->value;
              }
            if(positions.second==last)
              current=0;
            else
              {
                lower=tolower(*positions.second);
                if(lower!=*positions.second)
                  {
                    alternative=current->find_child_b(lower);
                    if(alternative!=0)
                      alternatives.push(std::make_pair(positions.second,alternative));
                  }
                current=current->find_child_b(*positions.second);
              }
          }
        else
          current=0;
        if((current==0)&&(result.second==0)&&(!alternatives.empty()))
          {
            current=alternatives.top().second;
            positions.second=alternatives.top().first;
            alternatives.pop();
          }
      }
    return result;
  }

  template<typename E,typename T,class L> template<typename forward_iterator>
    typename trie<E,T,L>::node* trie<E,T,L>::get_node(forward_iterator first,forward_iterator last)
  {
    if(sorted)
      throw std::logic_error("Elements cannot be inserted into the trie after it has been sorted");
    node* current=root;
    node* child=0;
    std::pair<label_iterator,forward_iterator> positions(current->label.begin(),first);
    while(true)
      {
        positions=current->get_common_prefix(positions.second,last);
        if(positions.first!=current->label.end())
          {
            current->split(positions.first);
            break;
          }
        if(positions.second==last)
          break;
        child=current->find_child_l(*positions.second);
        if(child==0)
          break;
        current=child;
      }
    if(positions.second==last)
      return current;
    else
      return current->add_child(positions.second,last);
  }
}
#endif
