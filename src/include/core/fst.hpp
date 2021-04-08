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

    std::vector<state> states;
    alphabet symbols;

    typedef std::vector<state>::const_iterator state_iterator;
    typedef std::vector<std::pair<std::string,symbol_id> > input_symbols;

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
      dest.push_back(input_symbols::value_type(name,symbols.id(name,1)));
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

  public:
    template<class input_iterator,class output_iterator> bool translate(input_iterator first,input_iterator last,output_iterator output) const;
  };

  template<class output_iterator>
  bool fst::do_translate(const input_symbols& input,output_iterator output) const
  {
    if(states.empty())
      return false;
    input_symbols::const_iterator pos=input.begin();
    if(pos==input.end())
      return false;
    arc_filter f(states.begin(),pos->second);
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
            if(states[path.back().get().target].is_final())
              break;
            else
              f=arc_filter(states.begin()+path.back().get().target,0);
          }
        else
          f=arc_filter(states.begin()+path.back().get().target,pos->second);
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
    if((pos!=input.end())||path.empty()||(!states[path.back().get().target].is_final()))
      return false;
    pos=input.begin();
    for(std::vector<arc_filter>::const_iterator it=path.begin();it!=path.end();++it)
      {
        if(it->get().osymbol!=0)
          {
            if(it->get().osymbol==1)
              {
              *output=pos->first;
              ++output;
              }
            else
              {
                *output=symbols.name(it->get().osymbol);
                ++output;
              }
          }
        if(it->get().isymbol!=0)
          ++pos;
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
