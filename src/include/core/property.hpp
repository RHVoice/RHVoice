/* Copyright (C) 2012, 2020  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_PROPERTY_HPP
#define RHVOICE_PROPERTY_HPP

#include <string>
#include <algorithm>
#include <sstream>
#include <locale>
#include <stdexcept>
#include <set>
#include <map>
#include "utf.hpp"
#include "str.hpp"

namespace RHVoice
{
  class abstract_property
  {
  public:
    virtual ~abstract_property()
    {
    }

    const std::string& get_name() const
    {
      return name;
    }

    virtual bool set_from_string(const std::string& s)=0;
    virtual void reset()=0;
    virtual bool is_set(bool recursive=false) const=0;

  protected:
    explicit abstract_property(const std::string& name_):
      name(name_)
    {
    }

  private:
    abstract_property(const abstract_property&);
    abstract_property& operator=(const abstract_property&);

    const std::string name;
  };

  template<typename T>
  class property: public abstract_property
  {
  protected:
    const T& get_value() const
    {
      if(value_set)
        return current_value;
      else
        {
          if(next)
            return next->get_value();
          else
            return default_value;
        }
    }

    void set_default_value(const T& val)
    {
      default_value=val;
    }

    property(const std::string& name,const T& default_val):
      abstract_property(name),
      default_value(default_val),
      current_value(default_val),
      value_set(false),
      next(0)
    {
    }

    bool set_value(const T& val)
    {
      T tmp;
      if(check_value(val,tmp)||(next&&next->check_value(val,tmp)))
        {
      current_value=tmp;
      value_set=true;
      return true;
        }
      else
        return false;
    }

  public:
    T get() const
    {
      return get_value();
    }

    operator T () const
    {
      return get_value();
    }

    bool is_set(bool recursive=false) const
    {
      return (value_set||(recursive&&next&&next->is_set(true)));
    }

    void reset()
    {
      current_value=default_value;
      value_set=false;
    }

    void default_to(const property& p)
    {
      next=&p;
    }

  private:
    T default_value;
    T current_value;
    bool value_set;
    const property* next;

    virtual bool check_value(const T& given_value,T& correct_value) const
    {
      correct_value=given_value;
      return true;
    }
  };

  template<typename T>
  class numeric_property: public property<T>
  {
  public:
    numeric_property(const std::string& name,T default_val,T min_val,T max_val):
      property<T>(name,default_val),
      min_value(min_val),
      max_value(max_val)
    {
      if(!((min_val<=default_val)&&(default_val<=max_val)))
        throw std::invalid_argument("Invalid range");
    }

    T get_min() const
    {
      return min_value;
}

    T get_max() const
    {
      return max_value;
}

    bool set_from_string(const std::string& s)
    {
      std::istringstream strm(s);
      strm.imbue(std::locale::classic());
      T val;
      return ((strm >> val)&&this->set_value(val));
    }

    numeric_property& operator=(T val)
    {
      this->set_value(val);
      return *this;
    }

  private:
    bool check_value(const T& given_value,T& correct_value) const
    {
      correct_value=std::max(min_value,std::min(max_value,given_value));
      return true;
    }

    T min_value,max_value;
  };

  class char_property: public property<utf8::uint32_t>
  {
  public:
    char_property(const std::string& name,utf8::uint32_t default_val):
      property<utf8::uint32_t>(name,default_val)
    {
    }

    bool set_from_string(const std::string& s)
    {
      std::string::const_iterator pos=s.begin();
      utf8::uint32_t cp=utf8::next(pos,s.end());
      return ((pos==s.end())&&this->set_value(cp));
    }

    char_property& operator=(utf8::uint32_t val)
    {
      this->set_value(val);
      return *this;
    }

  private:
    bool check_value(const utf8::uint32_t& given_value,utf8::uint32_t& correct_value) const
    {
      if(utf::is_valid(given_value))
        {
          correct_value=given_value;
          return true;
        }
      else
        return false;
    }
  };

  template<typename T>
  class enum_property: public property<T>
  {
  private:
    typedef std::map<std::string,T,str::less> name_map;

  public:
    enum_property(const std::string& name,T default_val):
      property<T>(name,default_val)
    {
    }

    bool set_from_string(const std::string& s)
    {
      #ifdef _MSC_VER
      name_map::const_iterator it=names_to_values.find(s);
      #else
      typename name_map::const_iterator it=names_to_values.find(s);
      #endif
      return ((it!=names_to_values.end())&&this->set_value(it->second));
    }

    enum_property& operator=(T val)
    {
      this->set_value(val);
      return *this;
    }

    void define(const std::string& name,T val)
    {
      #ifdef _MSC_VER
      names_to_values.insert(name_map::value_type(name,val));
      #else
      names_to_values.insert(typename name_map::value_type(name,val));
      #endif
    }

  private:
    name_map names_to_values;
  };

  class string_property: public property<std::string>
  {
  public:
    explicit string_property(const std::string& name):
      property<std::string>(name,std::string())
    {
    }

    bool set_from_string(const std::string& s)
    {
      return this->set_value(s);
    }

    string_property& operator=(const std::string& val)
    {
      this->set_value(val);
      return *this;
    }
  };

  class enum_string_property: public string_property
  {
  private:
    typedef std::set<std::string,str::less> range;
    range values;

  private:
    bool check_value(const std::string& given_value,std::string& correct_value) const
    {
      range::const_iterator it=values.find(given_value);
      if(it!=values.end())
        {
          correct_value=given_value;
          return true;
        }
      else
        return false;
    }

  public:
    explicit enum_string_property(const std::string& name):
    string_property(name)
    {
    }

    enum_string_property& operator=(const std::string& val)
    {
      this->set_value(val);
      return *this;
    }

    void define(const std::string& val)
    {
      values.insert(val);
    }
  };

  class bool_property: public enum_property<bool>
  {
  public:
    bool_property(const std::string& name,bool default_val):
      enum_property<bool>(name,default_val)
    {
      define("1",true);
      define("0",false);
      define("true",true);
      define("false",false);
      define("yes",true);
      define("no",false);
      define("on",true);
      define("off",false);
    }

    bool_property& operator=(bool val)
    {
      this->set_value(val);
      return *this;
    }
  };

  class charset_property: public property<std::set<utf8::uint32_t> >
  {
  private:
    typedef std::set<utf8::uint32_t> charset;

  public:
    charset_property(const std::string& name,const std::string& chars):
      property<charset>(name,charset(str::utf8_string_begin(chars),str::utf8_string_end(chars)))
    {
    }

    bool set_from_string(const std::string& s)
    {
      charset val(str::utf8_string_begin(s),str::utf8_string_end(s));
      return this->set_value(val);
    }

    bool includes(utf8::uint32_t c) const
    {
      const charset& val=get_value();
      return (val.find(c)!=val.end());
    }
  };

  // The enumeration must start from 0,
  //   and the first value must mean "default",
  // that is that the caller leaves the decision to the library.
  template<typename T>
  class c_enum_property: public enum_property<T>
  {
  public:
    c_enum_property(const std::string& name,T default_value):
      enum_property<T>(name,default_value)
    {
    }

    c_enum_property& operator=(T val)
    {
      if(val==0)
        this->reset();
      else
        this->set_value(val);
      return *this;
    }
  };

  class stringset_property: public property<std::set<std::string> >
  {
  private:
    typedef std::set<std::string> stringset;

  public:
    stringset_property(const std::string& name):
      property<stringset>(name,stringset())
    {
    }

    bool set_from_string(const std::string& s)
    {
      str::tokenizer<str::is_equal_to> tok(s,str::is_equal_to(','));
      stringset val(tok.begin(),tok.end());
      return this->set_value(val);
    }

    bool includes(const std::string& s) const
    {
      const stringset& val=get_value();
      return (val.find(s)!=val.end());
    }
  };
}
#endif
