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

#ifndef RHVOICE_ENGLISH_ID_HPP
#define RHVOICE_ENGLISH_ID_HPP

#include <atomic>
#include <algorithm>
#include "str.hpp"
#include "voice_profile.hpp"
#include <iostream>
namespace RHVoice {
class english_id
{
public:
  explicit english_id(const voice_profile& p);

  bool is_enabled() const
  {
    return enabled;
  }

  template<typename T>
  void add_word(T first, T last);

  int get_result() const;

private:
  int set_result(int r) const;

  int last_result() const
  {
    return was_english?1:0;
  }

  template<typename T> bool check_chars(T first, T last);

  const voice_profile& profile;
  language_list::const_iterator lang, eng;
  bool enabled{false};
  std::size_t word_count{0};
  std::size_t eng_count{0};
  std::size_t common_count{0};
  std::size_t unk_count{0};
  bool has_unique_letters{false};
  static std::atomic_bool was_english;
};

  template<typename T>
  bool english_id::check_chars(T first, T last)
  {
    for(T it=first;it!=last;++it)
      {
	auto c=*it;
	if(c>='a' && c<='z')
	  continue;
	if(c>='A' && c<='Z')
	  continue;
	    if(c=='\'')
	      continue;
	return false;
      }
    return true;
  }

  template<typename T>
  void english_id::add_word(T first, T last)
  {
    if(!enabled)
      return;
    std::vector<utf8::uint32_t> word(first, last);
    for(auto c: word)
      {
	if(lang->is_letter(c) && !eng->is_letter(c))
	  {
	    has_unique_letters=true;
	    ++word_count;
	    return;
	  }
      }
    auto not_punct=[](utf8::uint32_t c)-> bool {return !str::ispunct(c);};
    auto start=std::find_if(word.begin(), word.end(), not_punct);
    if(start==word.end())
      return;
    if(lang->is_letter(*start) || eng->is_letter(*start))
      ++word_count;
    else
      return;
    auto end=std::find_if(word.rbegin(), word.rend(), not_punct).base();
    std::vector<utf8::uint32_t> lword;
    std::transform(start, end, std::back_inserter(lword), str::to_lower());
    if(lang->get_instance().is_common_with_english(lword.begin(), lword.end()))
      {
	++eng_count;
	++common_count;
      }
    else if(eng->get_instance().is_in_vocabulary(lword.begin(), lword.end())){
      ++eng_count;
    }
    else if( lang->get_instance().has_vocabulary() && check_chars(lword.begin(), lword.end()) && !lang->get_instance().is_in_vocabulary(lword.begin(), lword.end())) {
      ++unk_count;
    }
  }
}
#endif
