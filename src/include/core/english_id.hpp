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


  const voice_profile& profile;
  language_list::const_iterator lang, eng;
  bool enabled{false};
  std::size_t word_count{0};
  std::size_t eng_count{0};
  std::size_t common_count{0};
  bool has_unique_letters{false};
  static std::atomic_bool was_english;
};


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
    if((end-start)==1 && eng->is_letter(*start))
      {
	++eng_count;
	++common_count;
	return;
      }
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
  }
}
#endif
