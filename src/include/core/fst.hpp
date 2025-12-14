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

#ifndef RHVOICE_FST_HPP
#define RHVOICE_FST_HPP

#include <stdint.h>
#include <iostream>
#include <utility>
#include <string>
#include <vector>
#include <map>
#include <iterator>
#include <algorithm>
#include <functional>
#include "utf8.h"
#include "exception.hpp"
#include "item.hpp"

namespace RHVoice
{
  class fst
  {
  public:
    explicit fst(const std::string& path);
    
  private:
    typedef uint32_t state_id;
    typedef uint16_t symbol_id;

    class symbol_not_found: public lookup_error
    {
    public:
      symbol_not_found():
        lookup_error("Symbol not found")
      {
      }
    };

    class alphabet
    {
    public:
      void load(std::istream& in);
      std::string name(symbol_id id) const;
      std::string name(symbol_id id,const std::string& default_name) const;
      symbol_id id(const std::string& name) const;
      symbol_id id(const std::string& name,symbol_id default_id) const;
    private:
      typedef std::map<std::string,symbol_id> symbol_map;
      std::vector<std::string> ids_to_names;
      symbol_map names_to_ids;
    };

    struct arc
    {
      state_id target;
      symbol_id isymbol,osymbol;
      arc():
        target(0),
        isymbol(0),
        osymbol(0)
      {
      }

      arc(state_id t,symbol_id i,symbol_id o):
        target(t),
        isymbol(i),
        osymbol(o)
      {
      }

      explicit arc(std::istream& in);
    };

    class state
    {
    private:
      struct compare_arcs: public std::binary_function<bool,const arc&,const arc&>
      {
        bool operator()(const arc& arc1,const arc& arc2) const
        {
          return (arc1.isymbol<arc2.isymbol);
        }
      };

    public:
      explicit state(std::istream& in);

      bool is_final() const
      {
        return final;
      }

      state():
        final(false)
      {
      }

      typedef std::vector<arc>::const_iterator arc_iterator;

      arc_iterator begin() const
      {
        return arcs.begin();
      }

      arc_iterator find_arc(symbol_id id) const
      {
        arc_iterator it=std::lower_bound(arcs.begin(),arcs.end(),arc(0,id,0),compare_arcs());
        return (((it!=arcs.end())&&(it->isymbol==id))?it:arcs.end());
      }

      arc_iterator end() const
      {
        return arcs.end();
      }
    private:
      bool final;
      std::vector<arc> arcs;
    };

    typedef state::arc_iterator arc_iterator;

struct part
{
  alphabet symbols;
  std::vector<state> states;
};

    std::vector<part> parts;

    typedef std::vector<state>::const_iterator state_iterator;
    typedef std::pair<std::string,symbol_id> symbol_name;
    typedef std::vector<symbol_name> input_symbols;

    class arc_filter
    {
    public:
      arc_filter(state_iterator sstate,symbol_id isymbol):
        source_state(sstate),
        current_arc(source_state->find_arc(isymbol))
      {
        if(current_arc==source_state->end())
          current_arc=source_state->find_arc(0);
      }

      const arc& get() const
      {
        return *current_arc;
      }

      void next();

      bool done() const
      {
        return (current_arc==source_state->end());
      }
    private:
      state_iterator source_state;
      arc_iterator current_arc;
    };

    template<class output_iterator> bool do_translate(const input_symbols& input,output_iterator output) const;
    void append_input_symbol(const std::string& name,input_symbols& dest) const
    {
      dest.push_back(input_symbols::value_type(name,parts.at(0).symbols.id(name,1)));
    }

    void append_input_symbol(utf8::uint32_t chr,input_symbols& dest) const
    {
      std::string name;
      utf8::append(chr,std::back_inserter(name));
      append_input_symbol(name,dest);
    }

    void append_input_symbol(const item& i,input_symbols& dest) const
    {
      append_input_symbol(i.get("name").as<std::string>(),dest);
    }

    std::vector<state> load_states(std::istream& f);
    bool do_translate(const part& p, const input_symbols& input, input_symbols& output) const;

  public:
    template<class input_iterator,class output_iterator> bool translate(input_iterator first,input_iterator last,output_iterator output) const;
  };

  template<class output_iterator>
  bool fst::do_translate(const input_symbols& input,output_iterator output) const
  {
    input_symbols isyms{input};
    input_symbols osyms;
      for(const auto& p: parts)
	{
	  if(!osyms.empty())
	    {
	      isyms=osyms;
	      osyms.clear();
	      for(auto& s: isyms)
		s.second=p.symbols.id(s.first, 1);
	    }
	  bool r=do_translate(p, isyms, osyms);
	  if(!r)
	    return false;
	}
    for(const auto& osym: osyms)
      {
	*output=osym.first;
	++output;
      }
    return true;
  }

  template<class input_iterator,class output_iterator>
  bool fst::translate(input_iterator first,input_iterator last,output_iterator output) const
  {
    input_symbols input;
    for(input_iterator it=first;it!=last;++it)
      {
        append_input_symbol(*it,input);
      }
    return do_translate(input,output);
  }
}
#endif
