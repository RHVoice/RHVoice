/* Copyright (C) 2021 Grzegorz Zlotowicz  <grzezlo@gmail.com> */

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
#include <algorithm>
#include <iterator>
#include "core/path.hpp"
#include "core/utterance.hpp"
#include "core/relation.hpp"
#include "core/item.hpp"
#include "core/polish.hpp"

namespace RHVoice
{
  namespace
  {
    char polish_vowel_latin_letters[12]={'a','A','e','E','i','I','o','O','u','U','y','Y'};
  }

  polish_info::polish_info(const std::string& data_path,const std::string& userdict_path):
    #ifdef WIN32
    present_as_english("present_as_english",false),
    #endif
    language_info("Polish",data_path,userdict_path)
  {
    set_alpha2_code("pl");
    set_alpha3_code("pol");
    register_letter_range('a',26);
    register_letter_range('A',26);
    register_letter_range(0x104,2); //Ąą
    register_letter_range(0x106,2); //Ćć
    register_letter_range(0x118,2); //Ęę
    register_letter_range(0x141,2); //Łł
    register_letter_range(0x143,2); //Ńń
    register_letter(0xd3); //Ó
    register_letter(0xf3); //ó
    for(std::size_t i=0;i<12;++i)
      register_vowel_letter(polish_vowel_latin_letters[i]);
    register_vowel_letter(0x104); //Ą
    register_vowel_letter(0x105); //ą
    register_vowel_letter(0x118); //Ę
    register_vowel_letter(0x119); //ę
    register_vowel_letter(0xd3); //Ó
    register_vowel_letter(0xf3); //ó
  }

  std::shared_ptr<language> polish_info::create_instance() const
  {
    return std::shared_ptr<language>(new polish(*this));
  }

  #ifdef WIN32
  void polish_info::do_register_settings(config& cfg,const std::string& prefix)
  {
    language_info::do_register_settings(cfg,prefix);
    cfg.register_setting(present_as_english,prefix);
  }
  #endif

  polish::polish(const polish_info& info_):
    language(info_),
    info(info_),
    g2p_fst(path::join(info_.get_data_path(),"g2p.fst")),
    lseq_fst(path::join(info_.get_data_path(),"lseq.fst"))
  {
  }

  std::vector<std::string> polish::get_word_transcription(const item& word) const
  {
    std::vector<std::string> transcription;
    const std::string& name=word.get("name").as<std::string>();
    if(word.has_feature("lseq"))
      lseq_fst.translate(str::utf8_string_begin(name),str::utf8_string_end(name),std::back_inserter(transcription));
    else
      g2p_fst.translate(str::utf8_string_begin(name),str::utf8_string_end(name),std::back_inserter(transcription));
    return transcription;
  }

  void polish::post_lex(utterance& u) const
  {
    relation& sylstruct_rel=u.get_relation("SylStructure");
    for(relation::iterator word_iter(sylstruct_rel.begin());word_iter!=sylstruct_rel.end();++word_iter)
      {
        item& last_syl=word_iter->last_child();
        if(last_syl.has_prev())
          last_syl.prev().set<std::string>("stress","1");
        else
          last_syl.set<std::string>("stress","1");
      }
  }
}
