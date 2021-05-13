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

#ifndef RHVOICE_STRESS_PATTERN_HPP
#define RHVOICE_STRESS_PATTERN_HPP

#include <bitset>
#include <string>
#include "item.hpp"

namespace RHVoice
{
  class stress_pattern
  {
  public:
    enum state_t
      {
        undefined,
        stressed,
        unstressed
      };

    stress_pattern():
      state(undefined)
    {
    }

    state_t get_state() const
    {
      return state;
    }

    void stress_syllable(int n)
    {
      if(n>0)
        do_stress_syllable(fwmask,n);
      else if(n<0)
        do_stress_syllable(bwmask,-n);
    }

    void unstress()
    {
      reset();
      state=unstressed;
    }

    void reset()
    {
      state=undefined;
      fwmask.reset();
      bwmask.reset();
    }

    void apply(item& word) const
    {
      if(state==undefined)
        return;
      item& word_with_syls=word.as("SylStructure");
      clear_word_stress(word_with_syls);
      if(state==unstressed)
        return;
      do_apply(word_with_syls.begin(),word_with_syls.end(),fwmask);
      do_apply(word_with_syls.rbegin(),word_with_syls.rend(),bwmask);
    }

  private:
    typedef std::bitset<32> stress_mask;

    void do_stress_syllable(stress_mask& mask,int n)
    {
      if(n<=mask.size())
        {
        mask.set(n-1);
        state=stressed;
        }
    }

    void clear_word_stress(item& word) const
    {
      for(item::iterator it=word.begin();it!=word.end();++it)
        it->set<std::string>("stress","0");
    }

    template<typename forward_iterator>
    void do_apply(forward_iterator first,forward_iterator last,const stress_mask& mask) const
    {
      std::size_t i=0;
      for(forward_iterator current=first;(current!=last)&&(i<mask.size());++current,++i)
        {
          if(mask.test(i))
            {
              item& syl=*current;
              syl.set<std::string>("stress","1");
            }
        }
    }

  state_t state;
    stress_mask fwmask,bwmask;
  };
}
#endif
