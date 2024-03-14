/* Copyright (C) 2023  Olga Yakovleva <olga@rhvoice.org> */

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


#include "core/english_id.hpp"
#include <iostream>
namespace RHVoice
{
  std::atomic_bool english_id::was_english(false);

  english_id::english_id(const voice_profile& p):
    profile(p)
  {
    if(profile.voice_count()!=2)
      return;
    auto l2=profile.get_voice(1)->get_language();
    if(l2->get_name()!="English")
      return;
    if(!l2->get_instance().has_vocabulary())
      return;
    auto l1=profile.primary()->get_language();
    if(!l1->get_instance().has_english_words())
      return;
    lang=l1;
    eng=l2;
    enabled=true;
  }

  int english_id::set_result(int r) const {
    if(r<0)
      return r;
    was_english=(r==1);
    return r;
  }

  int english_id::get_result() const
  {
    if(has_unique_letters)
      return set_result(0);
    if(word_count==0)
      return set_result(0);
    if(lang->get_instance().has_vocabulary()) {
      if((common_count<eng_count || unk_count>0)&&
	 (eng_count+unk_count)==word_count)
	return set_result(1);
      else
	return set_result(0);
    }
    const auto thr=word_count*0.5;
    if((eng_count-common_count)>thr)
      return set_result(1);
    else
      return set_result(0);
  }
}
