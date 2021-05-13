/* Copyright (C) 2013  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include <algorithm>
#include "core/speech_processor.hpp"

namespace RHVoice
{
  bool speech_processor::fill_input_buffer(sample_ptr& start,sample_ptr end)
  {
    if(start==end)
      return false;
    std::size_t desired_size=get_desired_input_size();
    if(fixed_size_input())
      {
        if(desired_size>input.size())
          {
            std::size_t count=std::min<std::size_t>(desired_size-input.size(),end-start);
            input.insert(input.end(),start,start+count);
            start+=count;
          }
      }
    else
      {
        input.insert(input.end(),start,end);
        start=end;
      }
    return (input.size()>=desired_size);
  }

  void speech_processor::process(sample_ptr samples,std::size_t count)
  {
    sample_ptr start=samples;
    sample_ptr end=start+count;
    while(fill_input_buffer(start,end))
      {
        on_input();
        input.clear();
        if(is_stopped())
          return;
        on_output();
        if(is_stopped())
          return;
        if(!next)
          continue;
        if(!insertion.empty())
          {
            next->insert(&insertion[0],insertion.size());
            insertion.clear();
            if(is_stopped())
              {
                output.clear();
                return;
              }
          }
        if(output.empty())
          continue;
        next->process(&output[0],output.size());
        output.clear();
        if(is_stopped())
          return;
      }
  }

  void speech_processor::finish()
  {
    if(!input.empty())
      {
        on_input();
        input.clear();
        if(is_stopped())
          return;
      }
    on_end_of_input();
    if(is_stopped())
      return;
    on_output();
    if(is_stopped())
      return;
    if(!next)
      {
        on_finished();
        return;
      }
    if(!insertion.empty())
      {
        next->insert(&insertion[0],insertion.size());
        insertion.clear();
        if(is_stopped())
          {
            output.clear();
            return;
          }
      }
    if(!output.empty())
      {
        next->process(&output[0],output.size());
        output.clear();
      }
    if(is_stopped())
      return;
    next->finish();
    if(is_stopped())
      return;
    on_finished();
  }

  void speech_processor::insert(sample_ptr samples,std::size_t count)
  {
    if(accepts_insertions())
      process(samples,count);
    else if(next)
      next->insert(samples,count);
  }
}
