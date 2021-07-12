/* Copyright (C) 2016, 2018, 2020  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include <list>
#include "core/path.hpp"
#include "core/utterance.hpp"
#include "core/relation.hpp"
#include "core/item.hpp"
#include "core/io.hpp"
#include "core/macedonian.hpp"

namespace RHVoice
{
  macedonian_info::macedonian_info(const std::string& data_path,const std::string& userdict_path):
    language_info("Macedonian",data_path,userdict_path)
  {
    set_alpha2_code("mk");
    set_alpha3_code("mkd");
    register_letter_range('a',26);
    register_letter_range('A',26);
    register_letter_range(1024,1);
    register_letter_range(1028,1);
    register_letter_range(1030,2);
    register_letter_range(1037,1);
    register_letter_range(1040,26);
    register_letter_range(1068,1);
    register_letter_range(1070,28);
    register_letter_range(1100,1);
    register_letter_range(1102,2);
    register_letter_range(1104,1);
    register_letter_range(1108,1);
    register_letter_range(1110,2);
    register_letter_range(1117,1);
    register_letter_range(1168,2);
    register_vowel_letter(1028);
    register_vowel_letter(1030);
    register_vowel_letter(1031);
    register_vowel_letter(1040);
    register_vowel_letter(1045);
    register_vowel_letter(1048);
    register_vowel_letter(1054);
    register_vowel_letter(1059);
    register_vowel_letter(1070);
    register_vowel_letter(1071);
    register_vowel_letter(1072);
    register_vowel_letter(1077);
    register_vowel_letter(1080);
    register_vowel_letter(1086);
    register_vowel_letter(1091);
    register_vowel_letter(1102);
    register_vowel_letter(1103);
    register_vowel_letter(1108);
    register_vowel_letter(1110);
    register_vowel_letter(1111);
  }

  std::shared_ptr<language> macedonian_info::create_instance() const
  {
    return std::shared_ptr<language>(new macedonian(*this));
  }

  macedonian::macedonian(const macedonian_info& info_):
    language(info_),
    info(info_),
    g2p_fst(path::join(info_.get_data_path(),"g2p.fst")),
    lseq_fst(path::join(info_.get_data_path(),"lseq.fst"))
  {
    try
      {
        stress_marks_fst.reset(new fst(path::join(info.get_data_path(),"stress_marks.fst")));
      }
    catch(const io::open_error& e)
      {
      }
  }

  std::vector<std::string> macedonian::get_word_transcription(const item& word) const
  {
    std::vector<std::string> transcription;
    const std::string& name=word.get("name").as<std::string>();
    if(word.has_feature("lseq"))
      {
        lseq_fst.translate(str::utf8_string_begin(name),str::utf8_string_end(name),std::back_inserter(transcription));
        return transcription;
      }
    std::vector<std::string> stressed;
    if(stress_marks_fst && stress_marks_fst->translate(str::utf8_string_begin(name),str::utf8_string_end(name),std::back_inserter(stressed)))
      {
        g2p_fst.translate(stressed.begin(),stressed.end(),std::back_inserter(transcription));
        return transcription;
      }
    g2p_fst.translate(str::utf8_string_begin(name),str::utf8_string_end(name),std::back_inserter(transcription));
    return transcription;
  }

  void macedonian::post_lex(utterance& u) const
  {
}
}
