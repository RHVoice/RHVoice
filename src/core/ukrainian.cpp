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
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#include <list>
#include "core/path.hpp"
#include "core/utterance.hpp"
#include "core/relation.hpp"
#include "core/item.hpp"
#include "core/io.hpp"
#include "core/ukrainian.hpp"

namespace RHVoice
{
  ukrainian_info::ukrainian_info(const std::string& data_path,const std::string& userdict_path):
    language_info("Ukrainian",data_path,userdict_path)
  {
    set_alpha2_code("uk");
    set_alpha3_code("ukr");
    register_letter_range(1028,1);
    register_letter_range(1030,2);
    register_letter_range(1040,26);
    register_letter_range(1068,1);
    register_letter_range(1070,28);
    register_letter_range(1100,1);
    register_letter_range(1102,2);
    register_letter_range(1108,1);
    register_letter_range(1110,2);
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
    register_sign('\'');
    register_sign(700);
    register_sign(8217);
    register_sign(8216);
  }

  std::shared_ptr<language> ukrainian_info::create_instance() const
  {
    return std::shared_ptr<language>(new ukrainian(*this));
  }

  ukrainian::ukrainian(const ukrainian_info& info_):
    language(info_),
    info(info_),
    g2p_fst(path::join(info_.get_data_path(),"g2p.fst")),
    untranslit_fst(path::join(info_.get_data_path(),"untranslit.fst")),
    lseq_fst(path::join(info_.get_data_path(),"lseq.fst")),
    stress_fst(path::join(info_.get_data_path(),"stress.fst")),
    stress_rules(path::join(info_.get_data_path(),"stress.fsm"),io::integer_reader<uint8_t>())
  {
    try
      {
        stress_marks_fst.reset(new fst(path::join(info.get_data_path(),"stress_marks.fst")));
      }
    catch(const io::open_error& e)
      {
      }
  }

  std::vector<std::string> ukrainian::get_word_transcription(const item& word) const
  {
    std::vector<std::string> transcription;
    const std::string& name=word.get("name").as<std::string>();
    const item& word_in_phrase=word.as("Phrase");
    if(word.has_feature("lseq")||(str::is_single_char(name)&&!word_in_phrase.has_next()&&!word_in_phrase.has_prev()))
      {
        lseq_fst.translate(str::utf8_string_begin(name),str::utf8_string_end(name),std::back_inserter(transcription));
        return transcription;
      }
    std::vector<std::string> stressed;
    if((stress_marks_fst && stress_marks_fst->translate(str::utf8_string_begin(name),str::utf8_string_end(name),std::back_inserter(stressed))) || stress_fst.translate(str::utf8_string_begin(name),str::utf8_string_end(name),std::back_inserter(stressed)))
      {
        g2p_fst.translate(stressed.begin(),stressed.end(),std::back_inserter(transcription));
        return transcription;
      }
    std::vector<utf8::uint32_t> letters;
    letters.push_back('#');
    letters.insert(letters.end(),str::utf8_string_begin(name),str::utf8_string_end(name));
    letters.push_back('#');
    rules<uint8_t>::match m(stress_rules,letters.begin(),letters.end());
    if(!m.empty())
      {
        std::size_t pos=m.pos(m.size()-1)+m.value(m.size()-1);
        letters.at(pos)=str::toupper(letters.at(pos));
        g2p_fst.translate(letters.begin()+1,letters.end()-1,std::back_inserter(transcription));
        return transcription;
      }
    g2p_fst.translate(str::utf8_string_begin(name),str::utf8_string_end(name),std::back_inserter(transcription));
    return transcription;
  }

  void ukrainian::post_lex(utterance& u) const
  {
    stress_monosyllabic_words(u);
}

  void ukrainian::decode_as_word(item& token,const std::string& token_name) const
  {
    std::string word_name;
    std::list<std::string> lowercase_letters;
    downcase_fst.translate(str::utf8_string_begin(token_name),str::utf8_string_end(token_name),std::back_inserter(lowercase_letters));
    untranslit_fst.translate(lowercase_letters.begin(),lowercase_letters.end(),str::append_string_iterator(word_name));
    item& word=token.append_child();
    word.set("name",word_name);
  }
}
