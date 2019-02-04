/* Copyright (C) 2019  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 2.1 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include "core/path.hpp"
#include "core/utterance.hpp"
#include "core/relation.hpp"
#include "core/item.hpp"
#include "core/brazilian_portuguese.hpp"

namespace RHVoice
{
  brazilian_portuguese_info::brazilian_portuguese_info(const std::string& data_path,const std::string& userdict_path):
    language_info("Brazilian-Portuguese",data_path,userdict_path)
  {
    set_alpha2_code("pt");
    set_alpha3_code("por");
    register_letter_range(65,26);
    register_letter_range(97,26);
    register_letter_range(192,4);
    register_letter_range(199,1);
    register_letter_range(201,2);
    register_letter_range(205,1);
    register_letter_range(211,3);
    register_letter_range(218,1);
    register_letter_range(220,1);
    register_letter_range(224,4);
    register_letter_range(231,1);
    register_letter_range(233,2);
    register_letter_range(237,1);
    register_letter_range(243,3);
    register_letter_range(250,1);
    register_letter_range(252,1);
    register_vowel_letter(65);
    register_vowel_letter(69);
    register_vowel_letter(73);
    register_vowel_letter(79);
    register_vowel_letter(85);
    register_vowel_letter(97);
    register_vowel_letter(101);
    register_vowel_letter(105);
    register_vowel_letter(111);
    register_vowel_letter(117);
    register_vowel_letter(192);
    register_vowel_letter(193);
    register_vowel_letter(194);
    register_vowel_letter(195);
    register_vowel_letter(201);
    register_vowel_letter(202);
    register_vowel_letter(205);
    register_vowel_letter(211);
    register_vowel_letter(212);
    register_vowel_letter(213);
    register_vowel_letter(218);
    register_vowel_letter(220);
    register_vowel_letter(224);
    register_vowel_letter(225);
    register_vowel_letter(226);
    register_vowel_letter(227);
    register_vowel_letter(233);
    register_vowel_letter(234);
    register_vowel_letter(237);
    register_vowel_letter(243);
    register_vowel_letter(244);
    register_vowel_letter(245);
    register_vowel_letter(250);
    register_vowel_letter(252);
  }

  smart_ptr<language> brazilian_portuguese_info::create_instance() const
  {
    return smart_ptr<language>(new brazilian_portuguese(*this));
  }

  brazilian_portuguese::brazilian_portuguese(const brazilian_portuguese_info& info_):
    language(info_),
    info(info_),
    g2p_fst(path::join(info_.get_data_path(),"g2p.fst")),
    lseq_fst(path::join(info_.get_data_path(),"lseq.fst"))
  {
  }

  std::vector<std::string> brazilian_portuguese::get_word_transcription(const item& word) const
  {
    std::vector<std::string> transcription;
    const std::string& name=word.get("name").as<std::string>();
    if(word.has_feature("lseq"))
      lseq_fst.translate(str::utf8_string_begin(name),str::utf8_string_end(name),std::back_inserter(transcription));
    else
      g2p_fst.translate(str::utf8_string_begin(name),str::utf8_string_end(name),std::back_inserter(transcription));
    return transcription;
  }
}
