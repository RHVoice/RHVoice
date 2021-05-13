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

#include <sstream>
#include "core/exception.hpp"
#include "core/dtree.hpp"

namespace RHVoice
{
  namespace
  {
    const unsigned int value_string=0;
    const unsigned int value_number=1;
    const unsigned int value_list=2;
    const unsigned int condition_equal=1;
    const unsigned int condition_less=2;
    const unsigned int condition_grater=3;
    const unsigned int condition_in=4;
    const std::string err_msg("Incorrect format of the decision tree file");

    inline unsigned int read_number(std::istream& in)
    {
      uint8_t n;
      if(!io::read_integer(in,n))
        throw file_format_error(err_msg);
      return n;
    }

    inline std::string read_string(std::istream& in)
    {
      std::string s;
      if(!io::read_string(in,s))
        throw file_format_error(err_msg);
      return s;
    }
  }

  dtree::num_equal::num_equal(unsigned int num):
    as_number(num)
  {
    std::ostringstream os;
    os << as_number;
    as_string=os.str();
  }

  bool dtree::num_equal::test(const value& val) const
  {
    if(val.empty())
      return (as_number==0);
    else
      {
        if(val.is<std::string>())
          return (val.as<std::string>()==as_string);
        else
          return (val.as<unsigned int>()==as_number);
      }
  }

  dtree::in_list::in_list(std::istream& in)
  {
    unsigned int size=read_number(in);
    if(size==0)
      throw file_format_error(err_msg);
    tests.reserve(size);
    for(unsigned int i=0;i<size;++i)
      {
        switch(read_number(in))
          {
          case value_string:
            tests.push_back(std::shared_ptr<condition>(new str_equal(read_string(in))));
            break;
          case value_number:
            tests.push_back(std::shared_ptr<condition>(new num_equal(read_number(in))));
            break;
          default:
            throw file_format_error(err_msg);
          }
      }
  }

  bool dtree::in_list::test(const value& val) const
  {
    for(std::vector<std::shared_ptr<condition> >::const_iterator it(tests.begin());it!=tests.end();++it)
      {
        if((*it)->test(val))
          return true;
      }
    return false;
  }

  dtree::leaf_node::leaf_node(std::istream& in)
  {
    unsigned int type=read_number(in);
    switch(type)
      {
      case value_string:
        answer=read_string(in);
        break;
      case value_number:
        answer=read_number(in);
        break;
      default:
        throw file_format_error(err_msg);
      }
  }

  dtree::internal_node::internal_node(std::istream& in,unsigned int qtype):
    feature_name(read_string(in))
  {
    unsigned int vtype=read_number(in);
    switch(qtype)
      {
      case condition_equal:
        switch(vtype)
          {
          case value_string:
            question.reset(new str_equal(read_string(in)));
            break;
          case value_number:
            question.reset(new num_equal(read_number(in)));
            break;
          default:
            throw file_format_error(err_msg);
          }
        break;
      case condition_less:
        if(vtype!=value_number)
          throw file_format_error(err_msg);
        question.reset(new less(read_number(in)));
        break;
      case condition_grater:
        if(vtype!=value_number)
          throw file_format_error(err_msg);
        question.reset(new grater(read_number(in)));
        break;
      case condition_in:
        if(vtype!=value_list)
          throw file_format_error(err_msg);
        question.reset(new in_list(in));
        break;
      default:
        throw file_format_error(err_msg);
      }
    unsigned int next_type=read_number(in);
    if(next_type==0)
      yes_node.reset(new leaf_node(in));
    else
      yes_node.reset(new internal_node(in,next_type));
    next_type=read_number(in);
    if(next_type==0)
      no_node.reset(new leaf_node(in));
    else
      no_node.reset(new internal_node(in,next_type));
  }

  const dtree::node* dtree::internal_node::get_next_node(const dtree::features& f) const
  {
    return (((question->test(f.eval(feature_name)))?yes_node:no_node).get());
  }

  void dtree::load(std::istream& in)
  {
    unsigned int type=read_number(in);
    if(type==0)
      root.reset(new leaf_node(in));
    else
      root.reset(new internal_node(in,type));
  }

  const value& dtree::predict(const dtree::features& f) const
  {
    const node* cur_node=root.get();
    while(!(cur_node->is_leaf()))
      {
        cur_node=cur_node->get_next_node(f);
      }
    return *(cur_node->get_answer());
  }
}
