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

#ifndef RHVOICE_LTS_HPP
#define RHVOICE_LTS_HPP

#include <stdint.h>
#include <string>
#include <vector>
#include <map>
#include <istream>

namespace RHVoice
{
  class lts
  {
  private:
    struct state
    {
      int8_t letter_index;
      union
      {
        uint32_t letter;
        uint8_t phone_ids[4];
      };
      uint16_t yes_transition,no_transition;

      state():
        letter_index(0),
        yes_transition(0),
        no_transition(0)
      {
        letter='\0';
      }

      explicit state(std::istream& in);
    };

  public:
    explicit lts(const std::string& file_path);
    template<typename input_iterator,typename output_iterator>
    void apply(input_iterator start,input_iterator end,output_iterator output) const;

  private:
    lts(const lts&);
    lts& operator=(const lts&);

    uint8_t context_window_size;

    typedef std::vector<state> model;
    typedef std::map<uint32_t,model> model_map;
    model_map rules;

    std::vector<std::string> phones;
  };

  template<typename input_iterator,typename output_iterator>
  void lts::apply(input_iterator start,input_iterator end,output_iterator output) const
  {
    std::vector<uint32_t> chars(context_window_size-1,'\0');
    chars.push_back('#');
    chars.insert(chars.end(),start,end);
    chars.push_back('#');
    chars.insert(chars.end(),context_window_size-1,'\0');
    std::vector<uint32_t>::iterator start_pos=chars.begin()+context_window_size;
    std::vector<uint32_t>::iterator end_pos=chars.end()-context_window_size;
    for(std::vector<uint32_t>::iterator pos=start_pos;pos!=end_pos;++pos)
      {
        model_map::const_iterator found=rules.find(*pos);
        if(found==rules.end())
          continue;
        const model& mod=found->second;
        model::const_iterator cur_state=mod.begin();
        while(true)
          {
            if(cur_state->letter_index==0)
              {
                for(uint8_t i=0;(i<4)&&(cur_state->phone_ids[i]!=0);++i)
                  {
                    *output=phones[cur_state->phone_ids[i]];
                    ++output;
                  }
                break;
              }
            else
              cur_state=mod.begin()+((pos[cur_state->letter_index]==cur_state->letter)?(cur_state->yes_transition):(cur_state->no_transition));
          }
      }
  }
}
#endif
