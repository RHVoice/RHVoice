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
#include "core/fst.hpp"
#include "core/exception.hpp"
#include "core/io.hpp"

namespace RHVoice
{
  namespace
  {
    const std::string err_msg("Incorrect format of the fst file");
  }

  void fst::alphabet::load(std::istream& in)
  {
    fst::symbol_id num_symbols;
    if(!io::read_integer(in,num_symbols))
      throw file_format_error(err_msg);
    ids_to_names.clear();
    names_to_ids.clear();
    ids_to_names.reserve(num_symbols);
    std::string name;
    for(fst::symbol_id i=0;i<num_symbols;++i)
      {
        if(!io::read_string(in,name))
          throw file_format_error(err_msg);
        ids_to_names.push_back(name);
        names_to_ids[name]=i+2;
      }
  }

  fst::arc::arc(std::istream& in)
  {
    io::read_integer(in,target);
    io::read_integer(in,isymbol);
    io::read_integer(in,osymbol);
    if(!in)
      throw file_format_error(err_msg);
  }

  fst::state::state(std::istream& in)
  {
    uint8_t final_flag;
    if(!io::read_integer(in,final_flag))
      throw file_format_error(err_msg);
    final=final_flag;
    uint32_t num_arcs;
    if(!io::read_integer(in,num_arcs))
      throw file_format_error(err_msg);
    if(num_arcs>0)
      {
        arcs.reserve(num_arcs);
        for(uint32_t i=0;i<num_arcs;++i)
          {
            arcs.push_back(arc(in));
          }
      }
  }

  fst::fst(const std::string& path)
  {
    std::ifstream f;
    io::open_ifstream(f,path,true);
    symbols.load(f);
    state_id num_states;
    if(!io::read_integer(f,num_states))
      throw file_format_error(err_msg);
    states.reserve(num_states);
    for(state_id i=0;i<num_states;++i)
      {
        states.push_back(state(f));
      }
  }

  std::string fst::alphabet::name(symbol_id id) const
  {
    if((id<2)||(id>=(ids_to_names.size()+2)))
      throw symbol_not_found();
    return ids_to_names[id-2];
  }

  std::string fst::alphabet::name(symbol_id id,const std::string& default_name) const
  {
    try
      {
        return name(id);
      }
    catch(const lookup_error&)
      {
        return default_name;
      }
  }

  fst::symbol_id fst::alphabet::id(const std::string& name) const
  {
    symbol_map::const_iterator it=names_to_ids.find(name);
    if(it==names_to_ids.end())
      throw symbol_not_found();
    return it->second;
  }

  fst::symbol_id fst::alphabet::id(const std::string& name,fst::symbol_id default_id) const
  {
    symbol_map::const_iterator it=names_to_ids.find(name);
    return ((it==names_to_ids.end())?default_id:(it->second));
  }

  void fst::arc_filter::next()
  {
    if(current_arc==source_state->end())
      return;
    symbol_id symbol=current_arc->isymbol;
    ++current_arc;
    if(current_arc==source_state->end())
      {
        if(symbol!=0)
          current_arc=source_state->find_arc(0);
      }
    else
      {
        if(current_arc->isymbol!=symbol)
          {
            if(symbol==0)
              current_arc=source_state->end();
            else
              current_arc=source_state->find_arc(0);
          }
      }
  }
}
