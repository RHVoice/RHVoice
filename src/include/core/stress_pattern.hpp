/* Copyright (C) 2013  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef RHVOICE_STRESS_PATTERN_HPP
#define RHVOICE_STRESS_PATTERN_HPP

#include <bitset>
#include <string>
#include "language.hpp"
#include "item.hpp"

namespace RHVoice
{
  class stress_pattern
  {
  public:
    bool defined() const
    {
      return (fwmask.any()||bwmask.any());
    }

    void stress_syllable(int n)
    {
      if(n>0)
        do_stress_syllable(fwmask,n);
      else if(n<0)
        do_stress_syllable(bwmask,-n);
    }

    template<typename forward_iterator>
    void stress_syllable_from_mark(forward_iterator start,forward_iterator pos,const language_info& lang)
    {
      if(lang.supports_stress_marks())
        stress_syllable(1+std::count_if(start,pos,is_vowel_letter(lang)));
    }

    void reset()
    {
      fwmask.reset();
      bwmask.reset();
    }

    void apply(item& word) const
    {
      if(!defined())
        return;
      item& word_with_syls=word.as("SylStructure");
      for(item::iterator it=word_with_syls.begin();it!=word_with_syls.end();++it)
        it->set<std::string>("stress","0");
      do_apply(word_with_syls.begin(),word_with_syls.end(),fwmask);
      do_apply(word_with_syls.rbegin(),word_with_syls.rend(),bwmask);
    }

  private:
    typedef std::bitset<32> stress_mask;

    void do_stress_syllable(stress_mask& mask,int n)
    {
      if(n<=mask.size())
        mask.set(n-1);
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

    stress_mask fwmask,bwmask;
  };
}
#endif
