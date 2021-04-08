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

#ifndef RHVOICE_RULES_HPP
#define RHVOICE_RULES_HPP

#include <stdint.h>
#include <utility>
#include <vector>
#include <queue>
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <memory>
#include "io.hpp"
#include "exception.hpp"

namespace RHVoice
{
  template<typename T>
  class rules
  {
    friend class match;
  private:
    rules(const rules&);
    rules& operator=(const rules&);

    struct arc
    {
      uint32_t symbol;
      uint32_t target;

      explicit arc(uint32_t s=0,uint32_t t=0):
        symbol(s),
        target(t)
      {
      }

      bool operator<(const arc& other) const
      {
        return (symbol<other.symbol);
      }

      bool operator==(const arc& other) const
      {
        return (symbol==other.symbol);
      }
    };

    struct record
    {
      uint8_t key_length;
      T value;

      record(uint8_t len,const T& val):
        key_length(len),
        value(val)
      {
      }

      explicit record(uint8_t len):
        key_length(len)
      {
      }
    };

    struct state
    {
      std::vector<arc> arcs;
      uint32_t fail,next;
      record* data;

      state():
        fail(0),
        next(0),
        data(0)
      {
      }
    };

    std::vector<state> states;
    bool built;

  public:
    rules():
      built(false)
    {
      states.push_back(state());
    }

    template<typename R> explicit rules(const std::string& file_path,R reader=R());

    ~rules()
    {
      #ifdef _MSC_VER
      for(std::vector<state>::iterator it=states.begin();it!=states.end();++it)
      #else
      for(typename std::vector<state>::iterator it=states.begin();it!=states.end();++it)
      #endif
        {
          delete it->data;
        }
    }

    class match
    {
    private:
      struct submatch
      {
        std::size_t pos;
        const record* data;

        submatch(std::size_t p,const record* d):
          pos(p),
          data(d)
        {
        }
      };

      std::vector<submatch> submatches;

    public:
      template<typename input_iterator> match(const rules& dict,input_iterator first,input_iterator last);

      bool empty() const
      {
        return submatches.empty();
      }

      std::size_t size() const
      {
        return submatches.size();
      }

      std::size_t pos(std::size_t i) const
      {
        return submatches.at(i).pos;
      }

      uint8_t length(std::size_t i) const
      {
        return submatches.at(i).data->key_length;
      }

      const T& value(std::size_t i) const
      {
        return submatches.at(i).data->value;
      }
    };

    template<typename input_iterator>
    void add(input_iterator first,input_iterator last,const T& value);
    void build();

    bool go_to(uint32_t& s,uint32_t c) const
    {
      const std::vector<arc>& arcs=states[s].arcs;
      arc a(c);
      #ifdef _MSC_VER
      std::pair<std::vector<arc>::const_iterator,std::vector<arc>::const_iterator> r;
      #else
      std::pair<typename std::vector<arc>::const_iterator,typename std::vector<arc>::const_iterator> r;
      #endif
      r=std::equal_range(arcs.begin(),arcs.end(),a);
      if(r.first==r.second)
        return (s==0);
      else
        {
          s=r.first->target;
          return true;
        }
    }
  };

  template<typename T> template<typename R>
  rules<T>::rules(const std::string& file_path,R reader)
  {
    std::ifstream f;
    io::open_ifstream(f,file_path,true);
    std::string err_msg("Error while loading an Aho-Corasick automaton from a file");
    uint32_t num_states=0;
    io::read_integer(f,num_states);
    if(num_states==0)
      throw file_format_error(err_msg);
    states.reserve(num_states);
    uint8_t num_arcs,key_length;
    arc a;
    for(uint32_t i=0;i<num_states;++i)
      {
        states.push_back(state());
        state& st=states.back();
        if(!io::read_integer(f,num_arcs))
          throw file_format_error(err_msg);
        if(num_arcs>0)
          {
            st.arcs.reserve(num_arcs);
            for(uint8_t j=0;j<num_arcs;++j)
              {
                if(!(io::read_integer(f,a.symbol)&&io::read_integer(f,a.target)))
                  throw file_format_error(err_msg);
                st.arcs.push_back(a);
              }
          }
        if(!(io::read_integer(f,st.fail)&&io::read_integer(f,st.next)&&io::read_integer(f,key_length)))
          throw file_format_error(err_msg);
        if(key_length!=0)
          {
            std::unique_ptr<record> data(new record(key_length));
            if(!reader(f,data->value))
              throw file_format_error(err_msg);
            st.data=data.release();
          }
      }
    built=true;
  }

  template<typename T> template<typename input_iterator>
  void rules<T>::add(input_iterator first,input_iterator last,const T& value)
  {
    if(first==last)
      throw std::invalid_argument("The input is empty");
    if(built)
      throw std::logic_error("All the strings should be added before building the fsm");
    arc a;
    uint32_t s=0;
    std::vector<uint32_t> chars(first,last);
    if(chars.size()>255)
      throw std::invalid_argument("The string is to long");
    for(std::vector<uint32_t>::const_iterator pos=chars.begin();pos!=chars.end();++pos)
      {
        a.symbol=*pos;
        #ifdef _MSC_VER
        std::vector<arc>::iterator it=std::find(states[s].arcs.begin(),states[s].arcs.end(),a);
        #else
        typename std::vector<arc>::iterator it=std::find(states[s].arcs.begin(),states[s].arcs.end(),a);
        #endif
        if(it==states[s].arcs.end())
          {
            a.target=states.size();
            states.push_back(state());
            states[s].arcs.push_back(a);
            s=a.target;
          }
        else
          s=it->target;
      }
    if(!states[s].data)
      states[s].data=new record(chars.size(),value);
  }

  template<typename T>
  void rules<T>::build()
  {
    if(built)
      throw std::logic_error("The fsm has already been built");
    #ifdef _MSC_VER
    for(std::vector<state>::iterator it=states.begin();it!=states.end();++it)
    #else
    for(typename std::vector<state>::iterator it=states.begin();it!=states.end();++it)
    #endif
      {
        std::sort(it->arcs.begin(),it->arcs.end());
      }
    std::queue<uint32_t> q;
    #ifdef _MSC_VER
    for(std::vector<arc>::const_iterator it=states[0].arcs.begin();it!=states[0].arcs.end();++it)
    #else
    for(typename std::vector<arc>::const_iterator it=states[0].arcs.begin();it!=states[0].arcs.end();++it)
    #endif
      {
        q.push(it->target);
      }
    uint32_t s,t,f,c;
    while(!q.empty())
      {
        s=q.front();
        q.pop();
        const std::vector<arc>& arcs=states[s].arcs;
        #ifdef _MSC_VER
        for(std::vector<arc>::const_iterator it=arcs.begin();it!=arcs.end();++it)
        #else
        for(typename std::vector<arc>::const_iterator it=arcs.begin();it!=arcs.end();++it)
        #endif
          {
            t=it->target;
            c=it->symbol;
            q.push(t);
            f=states[s].fail;
            while(!go_to(f,c))
              {
                f=states[f].fail;
              }
            states[t].fail=f;
            if(states[f].data)
              states[t].next=f;
            else
              states[t].next=states[f].next;
          }
      }
    built=true;
  }

  template<typename T> template<typename input_iterator>
  rules<T>::match::match(const rules<T>& dict,input_iterator first,input_iterator last)
  {
    if(first==last)
      throw std::invalid_argument("Empty input");
    if(!dict.built)
      throw std::logic_error("The fsm should be built before matching");
    const std::vector<state>& states=dict.states;
    std::vector<uint32_t> chars(first,last);
    std::vector<const record*> results(chars.size(),0);
    uint32_t s=0;
    uint32_t o;
    uint32_t c;
    const record* d;
    for(std::size_t i=0;i<chars.size();++i)
      {
        c=chars[i];
        while(!dict.go_to(s,c))
          {
            s=states[s].fail;
          }
        if(states[s].data)
          o=s;
        else
          o=states[s].next;
        while(o!=0)
          {
            d=states[o].data;
            results[i+1-d->key_length]=d;
            o=states[o].next;
          }
      }
    std::size_t i=0;
    while(i<results.size())
      {
        d=results[i];
        if(d)
          {
            submatches.push_back(submatch(i,d));
            i+=d->key_length;
          }
        else
          ++i;
      }
  }
}
#endif
