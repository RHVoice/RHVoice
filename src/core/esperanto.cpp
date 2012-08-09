/* Copyright (C) 2012  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

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
  smart_ptr<language> esperanto_info::create_instance() const
  {
    return smart_ptr<language>(new esperanto(*this));
  }

  esperanto::esperanto(const language_info& info):
    language(info),
    g2p_fst(path::join(info.get_data_path(),"g2p.fst")),
    untranslit_fst(path::join(info.get_data_path(),"untranslit.fst"))
  {
  }

  void esperanto::decode_as_word(item& token) const
  {
    const std::string& token_name=token.get("name").as<std::string>();
    std::list<std::string> lowercase_letters;
    downcase_fst.translate(str::utf8_string_begin(token_name),str::utf8_string_end(token_name),std::back_inserter(lowercase_letters));
    std::string word_name;
    untranslit_fst.translate(lowercase_letters.begin(),lowercase_letters.end(),str::append_string_iterator(word_name));
    item& word=token.append_child();
    word.set("name",word_name);
  }

  void esperanto::decode_as_letter_sequence(item& token) const
  {
    const std::string& token_name=token.get("name").as<std::string>();
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

  void esperanto::transcribe_word(item& word) const
  {
    const std::string& name=word.get("name").as<std::string>();
    g2p_fst.translate(str::utf8_string_begin(name),str::utf8_string_end(name),word.back_inserter());
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
