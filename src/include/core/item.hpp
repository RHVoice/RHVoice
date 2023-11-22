/* Copyright (C) 2012, 2019  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_ITEM_HPP
#define RHVOICE_ITEM_HPP

#include <string>
#include <map>
#include <memory>
#include <iterator>
#include <functional>
#include <stdexcept>
#include "exception.hpp"
#include "value.hpp"

namespace RHVoice
{
  class relation;

  class item_not_found: public lookup_error
  {
  public:
    explicit item_not_found():
      lookup_error("Item not found")
    {
    }
  };

  class duplicate_item: public exception
  {
  public:
    explicit duplicate_item():
      exception("Item already belongs to this relation")
    {
    }
  };

  class feature_not_found: public lookup_error
  {
  public:
    explicit feature_not_found(const std::string name):
      lookup_error("Feature not found: "+name)
    {
    }
  };

  class item
  {
    friend class relation;
    friend class iterator;
    friend class const_iterator;
  private:
    typedef std::map<std::string,item*> self_ref_map;
    typedef std::map<std::string,value> feature_map;

    struct shared_data
    {
      feature_map features;
      self_ref_map self_refs;
    };

    std::shared_ptr<shared_data> data;
    relation* const relation_ptr;
    item *next_item,*prev_item;
    item* const parent_item;
    item *first_child_item,*last_child_item;
    static const value empty_value;

    void init();

    explicit item(relation* owner):
      data(new shared_data),
      relation_ptr(owner),
      next_item(0),
      prev_item(0),
      parent_item(0),
      first_child_item(0),
      last_child_item(0)
    {
      init();
    }

    explicit item(item* parent):
      data(new shared_data),
      relation_ptr(parent->relation_ptr),
      next_item(0),
      prev_item(0),
      parent_item(parent),
      first_child_item(0),
      last_child_item(0)
    {
      init();
    }

    item(item& other,relation* owner):
      data(other.data),
      relation_ptr(owner),
      next_item(0),
      prev_item(0),
      parent_item(0),
      first_child_item(0),
      last_child_item(0)
    {
      init();
    }

    item(item& other,item* parent):
      data(other.data),
      relation_ptr(parent->relation_ptr),
      next_item(0),
      prev_item(0),
      parent_item(parent),
      first_child_item(0),
      last_child_item(0)
    {
      init();
    }

    ~item()
    {
    }

  public:
    const item& as(const std::string& relation_name) const
    {
      self_ref_map::const_iterator it(data->self_refs.find(relation_name));
      if(it==data->self_refs.end())
        throw item_not_found();
      return *(it->second);
    }

    item& as(const std::string& relation_name)
    {
      return const_cast<item&>(static_cast<const item&>(*this).as(relation_name));
    }

    const item* as_ptr(const std::string& relation_name) const
    {
      self_ref_map::const_iterator it(data->self_refs.find(relation_name));
      if(it==data->self_refs.end())
        return 0;
      return (it->second);
    }

    item* as_ptr(const std::string& relation_name)
    {
      return const_cast<item*>(static_cast<const item&>(*this).as_ptr(relation_name));
    }

    bool in(const std::string& relation_name) const
    {
      return (data->self_refs.find(relation_name)!=data->self_refs.end());
    }

    const relation& get_relation() const
    {
      return *relation_ptr;
    }

    relation& get_relation()
    {
      return *relation_ptr;
    }

    bool has_next() const
    {
      return (next_item!=0);
    }

    const item& next() const
    {
      check(next_item);
      return *next_item;
    }

    item& next()
    {
      return const_cast<item&>(static_cast<const item&>(*this).next());
    }

    const item* next_ptr() const
    {
      return next_item;
    }

    item* next_ptr()
    {
      return next_item;
    }

    bool has_prev() const
    {
      return (prev_item!=0);
    }

    const item& prev() const
    {
      check(prev_item);
      return *prev_item;
    }

    item& prev()
    {
      return const_cast<item&>(static_cast<const item&>(*this).prev());
    }

    const item* prev_ptr() const
    {
      return prev_item;
    }

    item* prev_ptr()
    {
      return prev_item;
    }

    bool has_parent() const
    {
      return (parent_item!=0);
    }

    const item& parent() const
    {
      check(parent_item);
      return *parent_item;
    }

    item& parent()
    {
      return const_cast<item&>(static_cast<const item&>(*this).parent());
    }

    const item* parent_ptr() const
    {
      return parent_item;
    }

    item* parent_ptr()
    {
      return parent_item;
    }

    bool has_children() const
    {
      return (first_child_item!=0);
    }

    const item& first_child() const
    {
      check(first_child_item);
      return *first_child_item;
    }

    item& first_child()
    {
      return const_cast<item&>(static_cast<const item&>(*this).first_child());
    }

    const item* first_child_ptr() const
    {
      return first_child_item;
    }

    item* first_child_ptr()
    {
      return first_child_item;
    }

    const item& last_child() const
    {
      check(last_child_item);
      return *last_child_item;
    }

    item& last_child()
    {
      return const_cast<item&>(static_cast<const item&>(*this).last_child());
    }

    const item* last_child_ptr() const
    {
      return last_child_item;
    }

    item* last_child_ptr()
    {
      return last_child_item;
    }

    const item* relative_ptr(const std::string& path) const;

    item* relative_ptr(const std::string& path)
    {
      return const_cast<item*>(static_cast<const item&>(*this).relative_ptr(path));
    }

    const item& relative(const std::string& path) const
    {
      const item* ptr=relative_ptr(path);
      check(ptr);
      return *ptr;
}

    item& relative(const std::string& path)
    {
      return const_cast<item&>(static_cast<const item&>(*this).relative(path));
    }

  private:
    item* append_item(item* other);
    item* prepend_item(item* other);

    static void check(const item* ptr)
    {
      if(ptr==0)
        throw item_not_found();
    }

    std::pair<std::string,std::string> split_feat_spec(const std::string& spec) const;

  public:
    item& append();
    item& append(item& other);
    item& prepend();
    item& prepend(item& other);
    item& append_child();
    item& append_child(item& other);
    item& prepend_child();
    item& prepend_child(item& other);
    void remove();
    void clear();

    template<typename T>
    void set(const std::string& name,const T& val)
    {
      data->features[name]=value(val);
    }

    const value& get(const std::string& name,bool return_empty=false) const
    {
      feature_map::const_iterator it(data->features.find(name));
      if(it==data->features.end())
        {
          if(return_empty)
            return empty_value;
          else
            throw feature_not_found(name);
        }
      else
        return it->second;
    }

    const std::string& get_name() const
    {
      return get("name").as<std::string>();
}

    bool has_feature(const std::string& name) const
    {
      feature_map::const_iterator it(data->features.find(name));
      return (it!=data->features.end());
    }

    value eval(const std::string& feature) const;
    value eval(const std::string& feature,const value& default_value) const;

    void replace_features(const item& other) {
      data->features=other.data->features;
    }

  class const_iterator: public std::iterator<std::bidirectional_iterator_tag,const item>
  {
  private:
    const relation* r;
    const item *c,*p;
  public:
    explicit const_iterator(const item* cur=0,const item* parent=0,const relation* rel=0):
      r(rel),
      c(cur),
      p(parent)
    {
    }

    const item& operator*() const
    {
      return *c;
    }

    const item* operator->() const
    {
      return c;
    }

    const_iterator& operator++()
    {
      c=c->next_item;
      return *this;
    }

    const_iterator operator++(int);
    const_iterator& operator--();
    const_iterator operator--(int);

    bool operator==(const const_iterator& other) const
    {
      return (c==other.c);
    }

    bool operator!=(const const_iterator& other) const
    {
      return !(*this==other);
    }
  };

  class iterator: public std::iterator<std::bidirectional_iterator_tag,item>
  {
  private:
    relation* r;
    item *c,*p;
  public:
    explicit iterator(item* cur=0,item* parent=0,relation* rel=0):
      r(rel),
      c(cur),
      p(parent)
    {
    }

    item& operator*()
    {
      return *c;
    }

    item* operator->()
    {
      return c;
    }

    iterator& operator++()
    {
      c=c->next_item;
      return *this;
    }

    iterator operator++(int);
    iterator& operator--();
    iterator operator--(int);

    bool operator==(const iterator& other) const
    {
      return (c==other.c);
    }

    bool operator!=(const iterator& other) const
    {
      return !(*this==other);
    }

    operator const_iterator() const
    {
      return const_iterator(c,p,r);
    }
  };

    iterator begin()
    {
      return iterator(first_child_item,this,relation_ptr);
    }

    const_iterator begin() const
    {
      return const_iterator(first_child_item,this,relation_ptr);
    }

    iterator end()
    {
      return iterator(0,this,relation_ptr);
    }

    const_iterator end() const
    {
      return const_iterator(0,this,relation_ptr);
    }

    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    reverse_iterator rbegin()
    {
      return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const
    {
      return const_reverse_iterator(end());
    }

    reverse_iterator rend()
    {
      return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const
    {
      return const_reverse_iterator(begin());
    }

    class back_insert_iterator: public std::iterator<std::output_iterator_tag,void,void,void,void>
    {
    public:
      back_insert_iterator():
        parent_item(0)
      {
      }

      explicit back_insert_iterator(item* i):
      parent_item(i)
      {
      }

      back_insert_iterator(const back_insert_iterator& other):
      parent_item(other.parent_item)
      {
      }

      back_insert_iterator& operator=(const back_insert_iterator& other)
      {
        parent_item=other.parent_item;
        return *this;
      }

      back_insert_iterator& operator=(item& value)
      {
        parent_item->append_child(value);
        return *this;
      }

      back_insert_iterator& operator=(const std::string& value)
      {
        parent_item->append_child().set("name",value);
        return *this;
      }

      back_insert_iterator& operator*()
      {
        return *this;
      }

      back_insert_iterator& operator++()
      {
        return *this;
      }

      back_insert_iterator& operator++(int)
      {
        return *this;
      }

    private:
      item* parent_item;
    };

    back_insert_iterator back_inserter()
    {
      return back_insert_iterator(this);
    }

    class front_insert_iterator: public std::iterator<std::output_iterator_tag,void,void,void,void>
    {
    public:
      front_insert_iterator():
        parent_item(0)
      {
      }

      explicit front_insert_iterator(item* i):
      parent_item(i)
      {
      }

      front_insert_iterator(const front_insert_iterator& other):
      parent_item(other.parent_item)
      {
      }

      front_insert_iterator& operator=(const front_insert_iterator& other)
      {
        parent_item=other.parent_item;
        return *this;
      }

      front_insert_iterator& operator=(item& value)
      {
        parent_item->prepend_child(value);
        return *this;
      }

      front_insert_iterator& operator=(const std::string& value)
      {
        parent_item->prepend_child().set("name",value);
        return *this;
      }

      front_insert_iterator& operator*()
      {
        return *this;
      }

      front_insert_iterator& operator++()
      {
        return *this;
      }

      front_insert_iterator& operator++(int)
      {
        return *this;
      }

    private:
      item* parent_item;
    };

    front_insert_iterator front_inserter()
    {
      return front_insert_iterator(this);
    }

    iterator get_iterator()
    {
      return iterator(this,parent_item,relation_ptr);
    }

    const_iterator get_iterator() const
    {
      return const_iterator(this,parent_item,relation_ptr);
    }

    reverse_iterator get_reverse_iterator()
    {
      return reverse_iterator(get_iterator());
    }

    const_reverse_iterator get_reverse_iterator() const
    {
      return const_reverse_iterator(get_iterator());
    }
  };

  template<typename T>
  struct feature_getter: public std::unary_function<const item&,T>
  {
    feature_getter(const std::string& feature_name):
      name(feature_name)
    {
    }

    T operator()(const item& i) const
    {
      return i.eval(name).template as<T>();
    }

  private:
    std::string name;
  };

  template<typename T>
  struct feature_equals: public std::unary_function<const item&,bool>
  {
    feature_equals(const std::string& feature_name,const T& feature_value):
      name(feature_name),
      val(feature_value)
    {
    }

      bool operator()(const item& i) const
    {
      return (i.eval(name).template as<T>()==val);
    }

  private:
    std::string name;
    T val;
  };

  template<typename T>
  class set_feature_iterator: public std::iterator<std::output_iterator_tag,void,void,void,void>
  {
  public:
    set_feature_iterator(const std::string& feature_name,const item::iterator& first,const item::iterator& last):
      name(feature_name),
      pos(first),
      end(last)
    {
    }

    set_feature_iterator& operator=(const T& val)
    {
      if(pos==end)
        throw std::out_of_range("No more items");
      pos->set(name,val);
      return *this;
    }

    set_feature_iterator& operator*()
    {
      return *this;
    }

    set_feature_iterator& operator++()
    {
      ++pos;
      return *this;
    }

    set_feature_iterator operator++(int)
    {
      set_feature_iterator tmp=*this;
      ++(*this);
      return tmp;
    }
  private:
    std::string name;
    item::iterator pos,end;
  };
}
#endif
