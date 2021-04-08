/* Copyright (C) 2012  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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
#include "core/esperanto.hpp"

namespace RHVoice
{
  namespace
  {
    char esperanto_vowel_letters[10]={'a','A','e','E','i','I','o','O','u','U'};
  }

  esperanto_info::esperanto_info(const std::string& data_path,const std::string& userdict_path):
    #ifdef WIN32
    present_as_english("present_as_english",false),
    #endif
    language_info("Esperanto",data_path,userdict_path)
  {
    set_alpha2_code("eo");
    set_alpha3_code("epo");
    register_letter_range('a',26);
    register_letter_range('A',26);
    register_letter_range(0x108,2);
    register_letter_range(0x11c,2);
    register_letter_range(0x124,2);
    register_letter_range(0x134,2);
    register_letter_range(0x15c,2);
    register_letter_range(0x16c,2);
    for(std::size_t i=0;i<10;++i)
      register_vowel_letter(esperanto_vowel_letters[i]);
  }

  std::shared_ptr<language> esperanto_info::create_instance() const
  {
    return std::shared_ptr<language>(new esperanto(*this));
  }

  #ifdef WIN32
  void esperanto_info::do_register_settings(config& cfg,const std::string& prefix)
  {
    language_info::do_register_settings(cfg,prefix);
    cfg.register_setting(present_as_english,prefix);
  }
  #endif

  esperanto::esperanto(const esperanto_info& info_):
    language(info_),
    info(info_),
    g2p_fst(path::join(info_.get_data_path(),"g2p.fst")),
    untranslit_fst(path::join(info_.get_data_path(),"untranslit.fst"))
  {
  }

  void esperanto::decode_as_word(item& token,const std::string& token_name) const
  {
    std::list<std::string> lowercase_letters;
    downcase_fst.translate(str::utf8_string_begin(token_name),str::utf8_string_end(token_name),std::back_inserter(lowercase_letters));
    std::string word_name;
    untranslit_fst.translate(lowercase_letters.begin(),lowercase_letters.end(),str::append_string_iterator(word_name));
    item& word=token.append_child();
    word.set("name",word_name);
  }

  void esperanto::decode_as_letter_sequence(item& token,const std::string& token_name) const
  {
    std::list<std::string> lowercase_letters;
    downcase_fst.translate(str::utf8_string_begin(token_name),str::utf8_string_end(token_name),std::back_inserter(lowercase_letters));
    std::list<std::string> original_letters;
    untranslit_fst.translate(lowercase_letters.begin(),lowercase_letters.end(),std::back_inserter(original_letters));
    std::list<std::string>::iterator cur=original_letters.begin();
    std::list<std::string>::iterator next=cur;
    while(cur!=original_letters.end())
      {
        ++next;
        spell_fst.translate(cur,next,token.back_inserter());
        cur=next;
      }
  }

  std::vector<std::string> esperanto::get_word_transcription(const item& word) const
  {
    std::vector<std::string> transcription;
    const std::string& name=word.get("name").as<std::string>();
    g2p_fst.translate(str::utf8_string_begin(name),str::utf8_string_end(name),std::back_inserter(transcription));
    return transcription;
  }

  void esperanto::post_lex(utterance& u) const
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
