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

#include "core/path.hpp"
#include "core/utterance.hpp"
#include "core/relation.hpp"
#include "core/item.hpp"
#include "core/vietnamese.hpp"

namespace RHVoice
{
namespace
{
  item* next_syl_token(item* cur)
  {
    if(cur->has_next())
      return nullptr;
    item& par=cur->parent();
    if(!par.has_next())
      return nullptr;
    item& tok=par.next().first_child();
    if(tok.get("pos").as<std::string>()!="word")
      return nullptr;
    return &tok;
  }

  void remove_consumed_syllable(item* syl)
  {
    item& par=syl->parent();
    item& prev_par=par.prev();
    syl->remove();
    for(const item& child: par) {
      prev_par.append_child().replace_features(child);
    }
    par.as("Token").remove();
    par.remove();
  }
}

  vietnamese_info::vietnamese_info(const std::string& data_path,const std::string& userdict_path):
    data_only_language_info(data_path,userdict_path)
  {
  }

  std::shared_ptr<language> vietnamese_info::create_instance() const
  {
    return std::shared_ptr<language>(new vietnamese(*this));
  }

  vietnamese::vietnamese(const vietnamese_info& info_):
    data_only_language(info_),
    info(info_),
    tokdict_fst(path::join(info_.get_data_path(),"tokdict.fst"))
  {
  }

  bool vietnamese::is_word_in_tok_dict(const std::string& word) const
  {
    std::vector<std::string> in, out;
    str::utf8explode(word, std::back_inserter(in));
    normalize(in.begin(), in.end(), std::back_inserter(out));
    in=out;
    if(tokdict_fst.translate(in.begin(), in.end(), std::back_inserter(out)))
			 return true;
    out.clear();
    downcase(in.begin(), in.end(), std::back_inserter(out));
    in=out;
    out.clear();
    if(tokdict_fst.translate(in.begin(), in.end(), std::back_inserter(out)))
      return true;
    return false;
  }
#include <iostream>
  void vietnamese::decode_as_word(item& token,const std::string& name) const
  {
    if(name.find("-")!=std::string::npos)
      {
	default_decode_as_word(token, name);
	return;
      }
    std::string search_string=name;
    std::string syl=name;
    std::string word=syl;
    bool in_dict=is_word_in_tok_dict(syl);
    bool unknown_syls=!in_dict;
    item* next_tok=&token;
    item* consumed=nullptr;
    while((next_tok=next_syl_token(next_tok))!=nullptr) {
      if(consumed)
	{
	  remove_consumed_syllable(consumed);
	  consumed=nullptr;
	}
      if(next_tok->has_feature("lang") && next_tok->get("lang").as<std::string>()!=get_info().get_name())
	break;
      syl=next_tok->get("name").as<std::string>();
      if(syl.find("-")!=std::string::npos)
	break;
      search_string+=("+"+syl);
      unknown_syls= unknown_syls && !is_word_in_tok_dict(syl);
      in_dict=is_word_in_tok_dict(search_string);
      if(in_dict || unknown_syls)
	{
	  word+=("-"+syl);
	  next_tok->set<verbosity_t>("verbosity", verbosity_silent);
	  consumed=next_tok;
	}
      else
	break;
    }
    if(consumed)
      {
	remove_consumed_syllable(consumed);
	consumed=nullptr;
      }
    default_decode_as_word(token, word);
  }
}
