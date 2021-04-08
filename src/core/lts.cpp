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

#include <fstream>
#include "core/io.hpp"
#include "core/exception.hpp"
#include "core/lts.hpp"

namespace RHVoice
{
  namespace
  {
    const std::string err_msg("Incorrect format of the lts file");
  }

  lts::state::state(std::istream& in):
    letter_index(0),
    letter('\0'),
    yes_transition(0),
    no_transition(0)
  {
    if(!io::read_integer(in,letter_index))
      throw file_format_error(err_msg);
    if(letter_index==0)
      {
        for(uint8_t i=0;i<4;++i)
          {
            if(!io::read_integer(in,phone_ids[i]))
              throw file_format_error(err_msg);
          }
      }
    else
      {
        if(!(io::read_integer(in,letter)&&
             io::read_integer(in,yes_transition)&&
             io::read_integer(in,no_transition)))
          throw file_format_error(err_msg);
      }
  }

  lts::lts(const std::string& file_path):
    context_window_size(0)
  {
    std::ifstream f_in;
    io::open_ifstream(f_in,file_path,true);
    if(!io::read_integer(f_in,context_window_size))
      throw file_format_error(err_msg);
    uint8_t num_phones;
    if(!io::read_integer(f_in,num_phones))
      throw file_format_error(err_msg);
    phones.reserve(num_phones);
    std::string phone;
    for(uint8_t i=0;i<num_phones;++i)
      {
        if(!io::read_string(f_in,phone))
          throw file_format_error(err_msg);
        phones.push_back(phone);
      }
    uint8_t num_models;
    if(!io::read_integer(f_in,num_models))
      throw file_format_error(err_msg);
    for(uint8_t i=0;i<num_models;++i)
      {
        uint32_t letter='\0';
        if(!io::read_integer(f_in,letter))
          throw file_format_error(err_msg);
        uint16_t num_states=0;
        if(!io::read_integer(f_in,num_states))
          throw file_format_error(err_msg);
        model& m=rules[letter];
        if(!m.empty())
          throw file_format_error(err_msg);
        m.reserve(num_states);
        for(uint16_t j=0;j<num_states;++j)
          {
            m.push_back(state(f_in));
          }
      }
  }
}
