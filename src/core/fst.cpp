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
    parts.push_back(part());
    parts[0].symbols.load(f);
    parts[0].states=load_states(f);
  uint8_t n{0};
    if(!io::read_integer(f, n) || n==0)
      return;
    parts.reserve(n+1);
    for(auto i=0;i<n;++i)
      {
	parts.push_back(part());
	parts.back().symbols.load(f);
	parts.back().states=load_states(f);
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

  std::vector<fst::state> fst::load_states(std::istream& f)
  {
        state_id num_states;
    if(!io::read_integer(f,num_states))
      throw file_format_error(err_msg);
    std::vector<state> states;
    states.reserve(num_states);
    for(state_id i=0;i<num_states;++i)
      {
        states.push_back(state(f));
      }
    return states;
  }

  bool fst::do_translate(const fst::part& p, const fst::input_symbols& input, fst::input_symbols& output) const
  {
    if(p.states.empty())
      return false;
    input_symbols::const_iterator pos=input.begin();
    if(pos==input.end())
      return false;
    arc_filter f(p.states.begin(),pos->second);
    if(f.done())
      return false;
    std::vector<arc_filter> path;
    path.push_back(f);
    if(f.get().isymbol!=0)
      ++pos;
    while(!path.empty())
      {
        if(pos==input.end())
          {
            if(p.states[path.back().get().target].is_final())
              break;
            else
              f=arc_filter(p.states.begin()+path.back().get().target,0);
          }
        else
          f=arc_filter(p.states.begin()+path.back().get().target,pos->second);
        if(f.done())
          {
            while(!path.empty())
              {
                if(path.back().get().isymbol!=0)
                  --pos;
                path.back().next();
                if(path.back().done())
                  path.pop_back();
                else
                  {
                    if(path.back().get().isymbol!=0)
                      ++pos;
                    break;
                  }
              }
          }
        else
          {
            path.push_back(f);
            if(f.get().isymbol!=0)
              ++pos;
          }
      }
    if((pos!=input.end())||path.empty()||(!p.states[path.back().get().target].is_final()))
      return false;
    pos=input.begin();
    for(std::vector<arc_filter>::const_iterator it=path.begin();it!=path.end();++it)
      {
        if(it->get().osymbol!=0)
          {
            if(it->get().osymbol==1)
              {
		output.push_back(*pos);
              }
            else
              {
		auto id=it->get().osymbol;
                output.push_back(symbol_name(p.symbols.name(id), id));
              }
          }
        if(it->get().isymbol!=0)
          ++pos;
      }
    return true;
  }
}
