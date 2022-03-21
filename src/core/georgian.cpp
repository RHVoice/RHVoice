/* Copyright (C) 2013  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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
#include "core/georgian.hpp"

namespace RHVoice
{
  namespace
  {
    bool can_start_harmonic_cluster(const item& seg)
    {
      const std::string type=seg.eval("ph_ctype").as<std::string>();
      if(!((type=="s")||(type=="f")||(type=="a")))
        return false;
      const std::string place=seg.eval("ph_cplace").as<std::string>();
      if(!((place=="b")||(place=="a")||(place=="p")))
        return false;
      return true;
    }

    bool can_end_harmonic_cluster(const item& seg)
    {
      const std::string place=seg.eval("ph_cplace").as<std::string>();
      return ((place=="v")||(place=="u"));
    }

    bool same_lar_spec(const item& seg1,const item& seg2)
    {
      if(seg1.eval("ph_cvox").as<std::string>()!=seg2.eval("ph_cvox").as<std::string>())
        return false;
      if(seg1.eval("ph_cglot").as<std::string>()!=seg2.eval("ph_cglot").as<std::string>())
        return false;
      return true;
    }

    struct hts_initial_in_harmonic_cluster: public feature_function
    {
      hts_initial_in_harmonic_cluster():
        feature_function("initial_in_harmonic_cluster")
      {
      }

        value eval(const item& seg) const
      {
        std::string result("x");
        if(can_start_harmonic_cluster(seg))
          {
            result="0";
            const item& s1=seg.as("Transcription");
            if(s1.has_next())
              {
                const item& s2=s1.next();
                if(can_end_harmonic_cluster(s2)&&same_lar_spec(s1,s2))
                  result="1";
              }
          }
        return result;
      }
    };

    struct hts_final_in_harmonic_cluster: public feature_function
    {
      hts_final_in_harmonic_cluster():
        feature_function("final_in_harmonic_cluster")
      {
      }

        value eval(const item& seg) const
      {
        std::string result("x");
        if(can_end_harmonic_cluster(seg))
          {
            result="0";
            const item& s2=seg.as("Transcription");
            if(s2.has_prev())
              {
                const item& s1=s2.prev();
                if(can_start_harmonic_cluster(s1)&&same_lar_spec(s1,s2))
                  result="1";
              }
          }
        return result;
      }
    };
  }

  georgian_info::georgian_info(const std::string& data_path,const std::string& userdict_path):
    language_info("Georgian",data_path,userdict_path)
  {
    set_alpha2_code("ka");
    set_alpha3_code("kat");
    register_letter_range(4304,33);
    register_vowel_letter(4304);
    register_vowel_letter(4308);
    register_vowel_letter(4312);
    register_vowel_letter(4317);
    register_vowel_letter(4323);
  }

  std::shared_ptr<language> georgian_info::create_instance() const
  {
    return std::shared_ptr<language>(new georgian(*this));
  }

  georgian::georgian(const georgian_info& info_):
    language(info_),
    info(info_),
    g2p_fst(path::join(info_.get_data_path(),"g2p.fst"))
  {
    hts_labeller& labeller=get_hts_labeller();
    labeller.define_feature(std::shared_ptr<feature_function>(new hts_initial_in_harmonic_cluster));
    labeller.define_feature(std::shared_ptr<feature_function>(new hts_final_in_harmonic_cluster));
  }

  std::vector<std::string> georgian::get_word_transcription(const item& word) const
  {
    std::vector<std::string> transcription;
    const std::string& name=word.get("name").as<std::string>();
    g2p_fst.translate(str::utf8_string_begin(name),str::utf8_string_end(name),std::back_inserter(transcription));
    return transcription;
  }
}
