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

#ifndef RHVOICE_DTREE_HPP
#define RHVOICE_DTREE_HPP

#include <stdint.h>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <memory>

#include "io.hpp"
#include "value.hpp"
#include "item.hpp"

namespace RHVoice
{
  class dtree
  {
  private:
    class features
    {
    public:
      features()
      {
      }

      virtual ~features()
      {
      }

      virtual value eval(const std::string& name) const=0;

    private:
      features(const features&);
      features& operator=(const features&);
    };

    class item_features: public features
    {
    public:
      explicit item_features(const item& i):
      item_ptr(&i)
      {
      }

      value eval(const std::string& name) const
      {
        return item_ptr->eval(name,value());
      }

    private:
      const item* item_ptr;
    };

    class explicit_features: public features
    {
    public:
      explicit explicit_features(const std::map<std::string,value>& m):
        feature_map_ptr(&m)
      {
      }

      value eval(const std::string& name) const
      {
        std::map<std::string,value>::const_iterator it=feature_map_ptr->find(name);
        return ((it==feature_map_ptr->end())?value():(it->second));
      }

    private:
      const std::map<std::string,value>* feature_map_ptr;
    };

    class node
    {
    public:
      node()
      {
      }

      virtual ~node()
      {
      }

      virtual const node* get_next_node(const features& f) const
      {
        return 0;
      }

      virtual const value* get_answer() const
      {
        return 0;
      }

      virtual bool is_leaf() const
      {
        return false;
      }

    private:
      node(const node&);
      node& operator=(const node&);
    };

    class leaf_node: public node
    {
    public:
      explicit leaf_node(std::istream& in);

      bool is_leaf() const
      {
        return true;
      }

      const value* get_answer() const
      {
        return &answer;
      }

    private:
      value answer;
    };

      class condition
      {
      public:
        virtual ~condition()
        {
        }

        virtual bool test(const value& val) const=0;
      };

    class less: public condition
    {
    public:
      explicit less(unsigned int num):
      number(num)
      {
      }

      bool test(const value& val) const
      {
        return (val.empty()?(number>0):(val.as<unsigned int>()<number));
      }

    private:
      unsigned int number;
    };

    class grater: public condition
    {
    public:
      explicit grater(unsigned int num):
      number(num)
      {
      }

      bool test(const value& val) const
      {
        return (val.empty()?false:(val.as<unsigned int>()>number));
      }

    private:
      unsigned int number;
    };

    class num_equal: public condition
    {
    public:
      explicit num_equal(unsigned int num);
      bool test(const value& val) const;

    private:
      unsigned int as_number;
      std::string as_string;
    };

    class str_equal: public condition
    {
    public:
      explicit str_equal(const std::string& s):
        str(s)
      {
      }

      bool test(const value& val) const
      {
        return (val.empty()?false:(str==val.as<std::string>()));
      }

    private:
      std::string str;
    };

    class in_list: public condition
    {
    public:
      explicit in_list(std::istream& in);
      bool test(const value& val) const;

    private:
      std::vector<std::shared_ptr<condition> > tests;
    };

    class internal_node: public node
    {
    public:
      internal_node(std::istream& in,unsigned int qtype);
      const node* get_next_node(const features& f) const;

    private:
      std::string feature_name;
      std::unique_ptr<condition> question;
      std::unique_ptr<node> yes_node,no_node;
    };

    std::unique_ptr<node> root;

    dtree(const dtree&);
    dtree& operator=(const dtree&);

    const value& predict(const features& f) const;
    void load(std::istream& in);

  public:
    explicit dtree(const std::string& file_path)
    {
      std::ifstream f;
      io::open_ifstream(f,file_path,true);
      load(f);
    }

    explicit dtree(std::istream& in)
    {
      load(in);
    }

    const value& predict(const item& i) const
    {
      std::unique_ptr<item_features> f(new item_features(i));
      return predict(*f);
    }

    const value& predict(const std::map<std::string,value>& m) const
    {
      std::unique_ptr<explicit_features> f(new explicit_features(m));
      return predict(*f);
    }
  };
}
#endif
