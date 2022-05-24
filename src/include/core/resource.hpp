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

#ifndef RHVOICE_RESOURCE_HPP
#define RHVOICE_RESOURCE_HPP

#include <utility>
#include <string>
#include <iterator>
#include <map>

#include "threading.hpp"
#include "str.hpp"
#include "config.hpp"
#include "path.hpp"

namespace RHVoice
{
  template<class T>
  class resource_info
  {
  public:
    virtual ~resource_info() = default;

    const std::string& get_data_path() const
    {
      return data_path;
    }

    const std::string& get_name() const
    {
      return name;
    }

    const T& get_instance() const;

    virtual void register_settings(config& cfg)
    {
    }

    virtual bool is_enabled() const
    {
      return true;
    }

  protected:
    resource_info() = default;

    void set_name(const std::string& name_)
    {
      name=name_;
    }

    void set_data_path(const std::string& data_path_)
    {
      data_path=data_path_;
    }

  private:
    resource_info(const resource_info&);
    resource_info& operator=(const resource_info&);

    virtual std::shared_ptr<T> create_instance() const=0;

    std::string name,data_path;
    mutable std::shared_ptr<T> instance;
    mutable threading::mutex instance_mutex;
  };

  template<class T>
  const T& resource_info<T>::get_instance() const
  {
    threading::lock instance_lock(instance_mutex);
    if(! instance)
      instance=create_instance();
    return *instance;
  }

  template<class T>
  class resource_list
  {
  protected:
    typedef std::pair<unsigned int,unsigned int> version_info;

  private:
    typedef std::map<std::string,std::shared_ptr<T>,str::less> container;
    typedef std::map<std::string,version_info> version_map;

    struct resource_enabled: public std::unary_function<const typename container::value_type&,bool>
    {
      bool operator()(const typename container::value_type& val) const
      {
        std::shared_ptr<T> res=val.second;
        return res->is_enabled();
      }
    };

  public:
    class const_iterator: public std::iterator<std::forward_iterator_tag,const T>
    {
    public:
      const_iterator()
      {
      }

      const_iterator(typename container::const_iterator pos_,typename container::const_iterator end_pos_):
        pos(pos_),
        end_pos(end_pos_)
      {
      }

      const T& operator*() const
      {
        return *(pos->second);
      }

      const T* operator->() const
      {
        return pos->second.get();
      }

      bool operator==(const const_iterator& other) const
      {
        return (pos==other.pos);
      }

      bool operator!=(const const_iterator& other) const
      {
        return !(*this==other);
      }

      const_iterator& operator++()
      {
        ++pos;
        pos=std::find_if(pos,end_pos,resource_enabled());
        return *this;
      }

      const_iterator operator++(int)
      {
        const_iterator tmp=*this;
        ++(*this);
        return tmp;
      }

    private:
      typename container::const_iterator pos,end_pos;
    };

    class iterator: public std::iterator<std::forward_iterator_tag,T>
    {
    public:
      iterator() = default;

      iterator(typename container::iterator pos_,typename container::iterator end_pos_):
        pos(pos_),
        end_pos(end_pos_)
      {
      }

      T& operator*()
      {
        return *(pos->second);
      }

      T* operator->()
      {
        return pos->second.get();
      }

      bool operator==(const iterator& other) const
      {
        return (pos==other.pos);
      }

      bool operator!=(const iterator& other) const
      {
        return !(*this==other);
      }

      iterator& operator++()
      {
        ++pos;
        pos=std::find_if(pos,end_pos,resource_enabled());
        return *this;
      }

      iterator operator++(int)
      {
        iterator tmp=*this;
        ++(*this);
        return tmp;
      }

      operator const_iterator() const
      {
        return const_iterator(pos,end_pos);
      }

    private:
      typename container::iterator pos,end_pos;
    };

    iterator begin()
    {
      typename container::iterator pos=std::find_if(elements.begin(),elements.end(),resource_enabled());
      return iterator(pos,elements.end());
    }

    const_iterator begin() const
    {
      typename container::const_iterator pos=std::find_if(elements.begin(),elements.end(),resource_enabled());
      return const_iterator(pos,elements.end());
    }

    iterator end()
    {
      return iterator(elements.end(),elements.end());
    }

    const_iterator end() const
    {
      return const_iterator(elements.end(),elements.end());
    }

    iterator find(const std::string& name)
    {
      typename container::iterator pos=elements.find(name);
      if(pos!=elements.end())
        {
          std::shared_ptr<T> r=pos->second;
          if(!r->is_enabled())
            pos=elements.end();
        }
      return iterator(pos,elements.end());
    }

    const_iterator find(const std::string& name) const
    {
      typename container::const_iterator pos=elements.find(name);
      if(pos!=elements.end())
        {
          std::shared_ptr<T> r=pos->second;
          if(!r->is_enabled())
            pos=elements.end();
        }
      return const_iterator(pos,elements.end());
    }

    bool empty() const
    {
      return (begin()==end());
    }

    void register_settings(config& cfg)
    {
      for(typename container::iterator it(elements.begin());it!=elements.end();++it)
        {
          it->second->register_settings(cfg);
        }
    }

  protected:
    resource_list() = default;

    ~resource_list() = default;

    bool can_add(const std::string& name,const version_info& ver) const
    {
      version_map::const_iterator it=versions.find(name);
      if(it==versions.end())
        return true;
      return (it->second<ver);
}

    void add(const std::shared_ptr<T>& obj,const version_info& ver)
    {
      if(!can_add(obj->get_name(),ver))
        return;
      elements[obj->get_name()]=obj;
      versions[obj->get_name()]=ver;
    }

  private:
    resource_list(const resource_list&);
    resource_list& operator=(const resource_list&);

    container elements;
    version_map versions;
  };

  class resource_description
  {
  public:
    std::string type;
    std::string data_path;
    bool_property data_only{"data_only", false};
    string_property name;
    numeric_property<unsigned int> format;
    numeric_property<unsigned int> revision;

    resource_description(const std::string& type_,const std::string& data_path_):
      type(type_),
      data_path(data_path_),
      name("name"),
      format("format",0,0,100),
      revision("revision",0,0,100)
    {
      config conf;
      if(type=="language")
        conf.register_setting(data_only);
      conf.register_setting(name);
      conf.register_setting(format);
      conf.register_setting(revision);
      conf.load(path::join(data_path,type+".info"));
    }

  private:
    resource_description(const resource_description&);
    resource_description& operator=(const resource_description&);
  };
}
#endif
