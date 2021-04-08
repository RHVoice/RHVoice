/* Copyright (C) 2017  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include "core/path.hpp"
#include "core/utterance.hpp"
#include "core/relation.hpp"
#include "core/item.hpp"
#include "core/tatar.hpp"

namespace RHVoice
{
  tatar_info::tatar_info(const std::string& data_path,const std::string& userdict_path):
    language_info("Tatar",data_path,userdict_path)
  {
    set_alpha2_code("tt");
    set_alpha3_code("tat");
    register_letter_range(1025,1);
    register_letter_range(1040,64);
    register_letter_range(1105,1);
    register_letter_range(1174,2);
    register_letter_range(1186,2);
    register_letter_range(1198,2);
    register_letter_range(1210,2);
    register_letter_range(1240,2);
    register_letter_range(1256,2);
    register_vowel_letter(1025);
    register_vowel_letter(1040);
    register_vowel_letter(1045);
    register_vowel_letter(1048);
    register_vowel_letter(1054);
    register_vowel_letter(1059);
    register_vowel_letter(1067);
    register_vowel_letter(1069);
    register_vowel_letter(1070);
    register_vowel_letter(1071);
    register_vowel_letter(1072);
    register_vowel_letter(1077);
    register_vowel_letter(1080);
    register_vowel_letter(1086);
    register_vowel_letter(1091);
    register_vowel_letter(1099);
    register_vowel_letter(1101);
    register_vowel_letter(1102);
    register_vowel_letter(1103);
    register_vowel_letter(1105);
    register_vowel_letter(1198);
    register_vowel_letter(1199);
    register_vowel_letter(1240);
    register_vowel_letter(1241);
    register_vowel_letter(1256);
    register_vowel_letter(1257);
  }

  std::shared_ptr<language> tatar_info::create_instance() const
  {
    return std::shared_ptr<language>(new tatar(*this));
  }

  tatar::tatar(const tatar_info& info_):
    language(info_),
    info(info_),
    g2p_fst(path::join(info_.get_data_path(),"g2p.fst")),
g2p_p_fst(path::join(info_.get_data_path(),"g2p_p.fst")),
    lseq_fst(path::join(info_.get_data_path(),"lseq.fst"))
  {
  }

  std::vector<std::string> tatar::get_word_transcription(const item& word) const
  {
    std::vector<std::string> transcription;
    const std::string& name=word.get("name").as<std::string>();
    if(word.has_feature("lseq"))
      lseq_fst.translate(str::utf8_string_begin(name),str::utf8_string_end(name),std::back_inserter(transcription));
    else
      {
        if(word.get_relation().get_utterance().has_voice())
          g2p_fst.translate(str::utf8_string_begin(name),str::utf8_string_end(name),std::back_inserter(transcription));
        else
          g2p_p_fst.translate(str::utf8_string_begin(name),str::utf8_string_end(name),std::back_inserter(transcription));
      }
    return transcription;
  }
}
