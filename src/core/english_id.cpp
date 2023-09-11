#include "core/english_id.hpp"

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
    was_english=(r==2);
    return r;
  }

  int english_id::get_result() const
  {
    if(has_unique_letters)
      return set_result(0);
    if(word_count==0)
      return last_result();
    const auto thr=word_count/2.0;
    if(eng_count<thr)
      return set_result(0);
    if(eng_count>thr)
      {
	if(common_count<eng_count)
	  return set_result(1);
	else
	  return set_result(0);
      }
    return last_result();
  }
}
