/* Copyright (C) 2021  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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
#include <algorithm>
#include <iterator>
#include "utf8.h"
#include "core/path.hpp"
#include "core/utterance.hpp"
#include "core/relation.hpp"
#include "core/item.hpp"
#include "core/io.hpp"
#include "core/data_only_language.hpp"

namespace RHVoice
{
  data_only_language_info::data_only_language_info(const std::string& data_path,const std::string& userdict_path):
    language_info(resource_description("language", data_path).name,data_path,userdict_path)
  {
    config cfg;
    cfg.register_setting(loc.language2);
    cfg.register_setting(loc.language3);
    cfg.register_setting(loc.country2);
    cfg.register_setting(loc.country3);
    cfg.register_setting(loc.ms_id);
    cfg.load(path::join(data_path, "locale.info"));
    set_alpha2_code(loc.language2);
    set_alpha3_code(loc.language3);
    std::ifstream gf;
    io::open_ifstream(gf, path::join(data_path, "graph.txt"));
    std::string gs;
    char t;
    utf8::uint32_t gc;
    while(gf >> gs)
      {
        auto it=gs.cbegin();
        gc=utf8::next(it, gs.cend());
        if(!(gf >> t))
          break;
        register_letter(gc);
        if(t=='v')
          register_vowel_letter(gc);
      }
  }

  std::string data_only_language_info::get_country() const
  {
    std::string res;
    auto c2=loc.country2.get();
    auto c3=loc.country3.get();
    if(!c3.empty())
      {
        res=c3;
        if(!c2.empty())
          res+='/';
      }
    if(!c2.empty())
      res+=c2;
    return res;
  }

  std::shared_ptr<language> data_only_language_info::create_instance() const
  {
    return std::shared_ptr<language>(new data_only_language(*this));
  }

  data_only_language::data_only_language(const data_only_language_info& info_):
    language(info_),
    info(info_),
    g2p_fst(path::join(info_.get_data_path(),"g2p.fst")),
    lseq_fst(path::join(info_.get_data_path(),"lseq.fst"))
  {
    try
      {
        g2g_fst.reset(new fst(path::join(info_.get_data_path(),"g2g.fst")));
      }
    catch(const io::open_error& e)
      {
      }
    try
      {
        lex_fst.reset(new fst(path::join(info_.get_data_path(),"lex.fst")));
      }
    catch(const io::open_error& e)
      {
      }
    try
      {
        gg2g_fst.reset(new fst(path::join(info_.get_data_path(),"gg2g.fst")));
      }
    catch(const io::open_error& e)
      {
      }
  }

  std::vector<std::string> data_only_language::get_word_transcription(const item& word) const
  {
    std::vector<std::string> transcription;
    std::string name=word.get("name").as<std::string>();
        if(word.has_feature("lseq"))
      {
        lseq_fst.translate(str::utf8_string_begin(name),str::utf8_string_end(name),std::back_inserter(transcription));
        return transcription;
      }
        if(lcfg.g2p_case)
          name=word.eval("cname", name).as<std::string>();
	std::vector<std::string> g2p_input;
	str::utf8explode(name, std::back_inserter(g2p_input));
	std::vector<std::string> tmp;
	if(g2g_fst)
	  {
	    const item& tok=word.as("TokStructure").parent();
	    const std::string& pos=tok.get("pos").as<std::string>();
	    if(pos=="word" && !tok.has_feature("userdict"))
	      if(g2g_fst->translate(g2p_input.begin(), g2p_input.end(), std::back_inserter(tmp)))
		g2p_input=tmp;
	  }
	if(lex_fst!=nullptr) {
	  if(lex_fst->translate(g2p_input.begin(), g2p_input.end(), std::back_inserter(tmp)))
	    return tmp;
}
	g2p_fst.translate(g2p_input.begin(),g2p_input.end(),std::back_inserter(transcription));
    return transcription;
  }

  void data_only_language::before_g2p(utterance& u) const
  {
    if(!gg2g_fst)
      return;
    const auto is_word=[] (const item& i) {return i.as("TokStructure").parent().get("pos").as<std::string>()=="word";};
    const auto is_not_word=[&is_word] (const item& i) {return !is_word(i);};
    const std::string wb{"#"};
    auto& phr_rel=u.get_relation("Phrase");
    std::vector<std::string> input, output;
    std::string name;
    for(auto& phr: phr_rel)
      {
	auto w1it=std::find_if(phr.begin(), phr.end(), is_word);
	while(w1it!=phr.end())
	  {
	    auto w2it=std::find_if(w1it, phr.end(), is_not_word);
	    input.clear();
	    output.clear();
	    for(auto it=w1it;it!=w2it;++it)
	      {
		name=it->get("name").as<std::string>();
		input.push_back(name);
	      }
	    gg2g_fst->translate(input.begin(), input.end(), std::back_inserter(output));
	    auto out_it=output.begin();
	    for(auto w_it=w1it;w_it!=w2it;++w_it)
	      {
		if(out_it==output.end())
		  throw std::runtime_error("Gg2g word count mismatch");
		name=*out_it;
		w_it->set("name", name);
		++out_it;
	      }
	    w1it=std::find_if(w2it, phr.end(), is_word);
	  }
      }
  }
}
