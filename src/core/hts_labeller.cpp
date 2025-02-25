/* Copyright (C) 2012, 2016  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include <sstream>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <locale>
#include <cmath>
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_iterators.hpp"
#include "core/str.hpp"
#include "core/item.hpp"
#include "core/io.hpp"
#include "core/language.hpp"
#include "core/voice.hpp"
#include "core/hts_labeller.hpp"

namespace RHVoice
{
  namespace
  {
    inline bool is_silence(const item& seg)
    {
      return seg.get("name").as<std::string>()=="pau";
    }

    const value x(std::string("x"));
    const value zero(std::string("0"));
    struct hts_none: public feature_function
    {
      hts_none():
        feature_function("none")
      {
      }

      value eval(const item& seg) const
      {
        return x;
      }
    };


    struct hts_prev_prev_name: public feature_function
    {
      hts_prev_prev_name():
        feature_function("prev_prev_name")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval("p.p.name",x);
      }
    };

    struct hts_prev_name: public feature_function
    {
      hts_prev_name():
        feature_function("prev_name")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval("p.name",x);
      }
    };

    struct hts_name: public feature_function
    {
      hts_name():
        feature_function("name")
      {
      }

      value eval(const item& seg) const
      {
        return seg.get("name");
      }
    };

    struct hts_next_name: public feature_function
    {
      hts_next_name():
        feature_function("next_name")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval("n.name",x);
      }
    };

    struct hts_next_next_name: public feature_function
    {
      hts_next_next_name():
        feature_function("next_next_name")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval("n.n.name",x);
      }
    };

    struct hts_pos_in_syl_fw: public feature_function
    {
      hts_pos_in_syl_fw():
        feature_function("pos_in_syl_fw")
      {
      }

      value eval(const item& seg) const
      {
        return is_silence(seg)?x:(seg.eval("pos_in_syl").as<unsigned int>()+1);
      }
    };

    struct hts_pos_in_syl_bw: public feature_function
    {
      hts_pos_in_syl_bw():
        feature_function("pos_in_syl_bw")
      {
      }

      value eval(const item& seg) const
      {
        return is_silence(seg)?x:(seg.eval("R:SylStructure.parent.syl_numphones").as<unsigned int>()-seg.eval("pos_in_syl").as<unsigned int>());
      }
    };

    struct hts_prev_syl_stress: public feature_function
    {
      hts_prev_syl_stress():
        feature_function("prev_syl_stress")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval(is_silence(seg)?"p.R:SylStructure.parent.stress":"R:SylStructure.parent.R:Syllable.p.stress",zero);
      }
    };

    struct hts_prev_syl_accented: public feature_function
    {
      hts_prev_syl_accented():
        feature_function("prev_syl_accented")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval(is_silence(seg)?"p.R:SylStructure.parent.accented":"R:SylStructure.parent.R:Syllable.p.accented",zero);
      }
    };

    struct hts_prev_prev_syl_lex_tone: public feature_function
    {
      hts_prev_prev_syl_lex_tone():
        feature_function("prev_prev_syl_lex_tone")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval(is_silence(seg)?"p.R:SylStructure.parent.R:PhraseSylStructure.p.lex_tone":"R:SylStructure.parent.R:PhraseSylStructure.p.p.lex_tone",x);
      }
    };

    struct hts_prev_syl_lex_tone: public feature_function
    {
      hts_prev_syl_lex_tone():
        feature_function("prev_syl_lex_tone")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval(is_silence(seg)?"p.R:SylStructure.parent.lex_tone":"R:SylStructure.parent.R:PhraseSylStructure.p.lex_tone",x);
      }
    };

    struct hts_prev_syl_length: public feature_function
    {
      hts_prev_syl_length():
        feature_function("prev_syl_length")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval(is_silence(seg)?"p.R:SylStructure.parent.syl_numphones":"R:SylStructure.parent.R:Syllable.p.syl_numphones",zero);
      }
    };

    struct hts_prev_syl_coda_length: public feature_function
    {
      hts_prev_syl_coda_length():
        feature_function("prev_syl_coda_length")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval(is_silence(seg)?"p.R:SylStructure.parent.syl_codasize":"R:SylStructure.parent.R:Syllable.p.syl_codasize",zero);
      }
    };

    struct hts_syl_stress: public feature_function
    {
      hts_syl_stress():
        feature_function("syl_stress")
      {
      }

      value eval(const item& seg) const
      {
        return is_silence(seg)?x:seg.eval("R:SylStructure.parent.stress");
      }
    };

    struct hts_syl_accented: public feature_function
    {
      hts_syl_accented():
        feature_function("syl_accented")
      {
      }

      value eval(const item& seg) const
      {
        return is_silence(seg)?x:seg.eval("R:SylStructure.parent.accented");
      }
    };

    struct hts_syl_length: public feature_function
    {
      hts_syl_length():
        feature_function("syl_length")
      {
      }

      value eval(const item& seg) const
      {
        return is_silence(seg)?x:seg.eval("R:SylStructure.parent.syl_numphones");
      }
    };

    struct hts_syl_coda_length: public feature_function
    {
      hts_syl_coda_length():
        feature_function("syl_coda_length")
      {
      }

      value eval(const item& seg) const
      {
        return is_silence(seg)?x:seg.eval("R:SylStructure.parent.syl_codasize");
      }
    };

    struct hts_syl_lex_tone: public feature_function
    {
      hts_syl_lex_tone():
        feature_function("syl_lex_tone")
      {
      }

      value eval(const item& seg) const
      {
        return is_silence(seg)?x:seg.eval("R:SylStructure.parent.lex_tone");
      }
    };

    struct hts_syl_pos_type: public feature_function
    {
      hts_syl_pos_type():
        feature_function("syl_pos_type")
      {
      }

      value eval(const item& seg) const
      {
        return is_silence(seg)?x:(seg.eval("R:SylStructure.parent.syl_pos_type"));
      }
    };


    struct hts_syl_pos_in_word_fw: public feature_function
    {
      hts_syl_pos_in_word_fw():
        feature_function("syl_pos_in_word_fw")
      {
      }

      value eval(const item& seg) const
      {
        return is_silence(seg)?x:(seg.eval("R:SylStructure.parent.pos_in_word").as<unsigned int>()+1);
      }
    };

    struct hts_syl_pos_in_word_bw: public feature_function
    {
      hts_syl_pos_in_word_bw():
        feature_function("syl_pos_in_word_bw")
      {
      }

      value eval(const item& seg) const
      {
        return is_silence(seg)?x:(seg.eval("R:SylStructure.parent.parent.word_numsyls").as<unsigned int>()-seg.eval("R:SylStructure.parent.pos_in_word").as<unsigned int>());
      };
    };

    struct hts_syl_pos_in_phrase_fw: public feature_function
    {
      hts_syl_pos_in_phrase_fw():
        feature_function("syl_pos_in_phrase_fw")
      {
      }

      value eval(const item& seg) const
      {
        return is_silence(seg)?x:(seg.eval("R:SylStructure.parent.syl_in").as<unsigned int>()+1);
      }
    };

    struct hts_rel_syl_in: public feature_function
    {
      hts_rel_syl_in():
        feature_function("rel_syl_in")
      {
      }

      value eval(const item& seg) const
      {
	if(is_silence(seg))
	  return x;
        double v=seg.eval("R:SylStructure.parent.syl_in").as<unsigned int>();
	double n=seg.eval("R:SylStructure.parent.parent.R:Phrase.parent.phrase_numsyls").as<unsigned int>();
      double rv=v/n*10;
      return static_cast<unsigned int>(std::round(rv));
      }
    };

    struct hts_rel_syl_out: public feature_function
    {
      hts_rel_syl_out():
        feature_function("rel_syl_out")
      {
      }

      value eval(const item& seg) const
      {
	if(is_silence(seg))
	  return x;
        double v=seg.eval("R:SylStructure.parent.syl_out").as<unsigned int>();
	double n=seg.eval("R:SylStructure.parent.parent.R:Phrase.parent.phrase_numsyls").as<unsigned int>();
      double rv=v/n*10;
      return static_cast<unsigned int>(std::round(rv));
      }
    };

    struct hts_syl_pos_in_phrase_bw: public feature_function
    {
      hts_syl_pos_in_phrase_bw():
        feature_function("syl_pos_in_phrase_bw")
      {
      }

      value eval(const item& seg) const
      {
        return is_silence(seg)?x:(seg.eval("R:SylStructure.parent.syl_out").as<unsigned int>()+1);
      }
    };

    struct hts_syl_break: public feature_function
    {
      hts_syl_break():
        feature_function("syl_break")
      {
      }

      value eval(const item& seg) const
      {
        return is_silence(seg)?x:seg.eval("R:SylStructure.parent.syl_break");
      }
    };

    struct hts_num_stressed_syls_in_phrase_before_this_syl: public feature_function
    {
      hts_num_stressed_syls_in_phrase_before_this_syl():
        feature_function("num_stressed_syls_in_phrase_before_this_syl")
      {
      }

      value eval(const item& seg) const
      {
        return is_silence(seg)?x:(seg.eval("R:SylStructure.parent.ssyl_in").as<unsigned int>()+1);
      }
    };

    struct hts_num_stressed_syls_in_phrase_after_this_syl: public feature_function
    {
      hts_num_stressed_syls_in_phrase_after_this_syl():
        feature_function("num_stressed_syls_in_phrase_after_this_syl")
      {
      }

      value eval(const item& seg) const
      {
        return is_silence(seg)?x:(seg.eval("R:SylStructure.parent.ssyl_out").as<unsigned int>()+1);
      }
    };

    struct hts_num_accented_syls_in_phrase_before_this_syl: public feature_function
    {
      hts_num_accented_syls_in_phrase_before_this_syl():
        feature_function("num_accented_syls_in_phrase_before_this_syl")
      {
      }

      value eval(const item& seg) const
      {
        return is_silence(seg)?x:(seg.eval("R:SylStructure.parent.asyl_in").as<unsigned int>()+1);
      }
    };

    struct hts_num_accented_syls_in_phrase_after_this_syl: public feature_function
    {
      hts_num_accented_syls_in_phrase_after_this_syl():
        feature_function("num_accented_syls_in_phrase_after_this_syl")
      {
      }

      value eval(const item& seg) const
      {
        return is_silence(seg)?x:(seg.eval("R:SylStructure.parent.asyl_out").as<unsigned int>()+1);
      }
    };

    struct hts_dist_to_prev_stressed_syl_in_phrase: public feature_function
    {
      hts_dist_to_prev_stressed_syl_in_phrase():
        feature_function("dist_to_prev_stressed_syl_in_phrase")
      {
      }

      value eval(const item& seg) const
      {
        if(is_silence(seg))
          return x;
        item::const_reverse_iterator syl_pos=seg.as("SylStructure").parent().as("Syllable").get_reverse_iterator();
        item::const_reverse_iterator first_syl_in_phrase_pos=seg.as("SylStructure").parent().parent().as("Phrase").parent().first_child().as("SylStructure").first_child().as("Syllable").get_reverse_iterator();
        item::const_reverse_iterator stressed_syl_pos=std::find_if(syl_pos,first_syl_in_phrase_pos,feature_equals<std::string>("stress","1"));
        unsigned int result=(stressed_syl_pos==first_syl_in_phrase_pos)?0:(std::distance(syl_pos,stressed_syl_pos)+1);
        return result;
      }
    };

    struct hts_dist_to_next_stressed_syl_in_phrase: public feature_function
    {
      hts_dist_to_next_stressed_syl_in_phrase():
        feature_function("dist_to_next_stressed_syl_in_phrase")
      {
      }

      value eval(const item& seg) const
      {
        if(is_silence(seg))
          return x;
        item::const_iterator syl_pos=seg.as("SylStructure").parent().as("Syllable").get_iterator();
        item::const_iterator first_syl_in_next_phrase_pos=++(syl_pos->as("SylStructure").parent().as("Phrase").parent().last_child().as("SylStructure").last_child().as("Syllable").get_iterator());
        item::const_iterator stressed_syl_pos=std::find_if(++item::const_iterator(syl_pos),first_syl_in_next_phrase_pos,feature_equals<std::string>("stress","1"));
        unsigned int result=(stressed_syl_pos==first_syl_in_next_phrase_pos)?0:std::distance(syl_pos,stressed_syl_pos);
        return result;
      }
    };

    struct hts_dist_to_prev_accented_syl_in_phrase: public feature_function
    {
      hts_dist_to_prev_accented_syl_in_phrase():
        feature_function("dist_to_prev_accented_syl_in_phrase")
      {
      }

      value eval(const item& seg) const
      {
        if(is_silence(seg))
          return x;
        item::const_reverse_iterator syl_pos=seg.as("SylStructure").parent().as("Syllable").get_reverse_iterator();
        item::const_reverse_iterator first_syl_in_phrase_pos=seg.as("SylStructure").parent().parent().as("Phrase").parent().first_child().as("SylStructure").first_child().as("Syllable").get_reverse_iterator();
        item::const_reverse_iterator accented_syl_pos=std::find_if(syl_pos,first_syl_in_phrase_pos,feature_equals<std::string>("accented","1"));
        unsigned int result=(accented_syl_pos==first_syl_in_phrase_pos)?0:(std::distance(syl_pos,accented_syl_pos)+1);
        return result;
      }
    };

    struct hts_dist_to_next_accented_syl_in_phrase: public feature_function
    {
      hts_dist_to_next_accented_syl_in_phrase():
        feature_function("dist_to_next_accented_syl_in_phrase")
      {
      }

      value eval(const item& seg) const
      {
        if(is_silence(seg))
          return x;
        item::const_iterator syl_pos=seg.as("SylStructure").parent().as("Syllable").get_iterator();
        item::const_iterator first_syl_in_next_phrase_pos=++(syl_pos->as("SylStructure").parent().as("Phrase").parent().last_child().as("SylStructure").last_child().as("Syllable").get_iterator());
        item::const_iterator accented_syl_pos=std::find_if(++item::const_iterator(syl_pos),first_syl_in_next_phrase_pos,feature_equals<std::string>("accented","1"));
        unsigned int result=(accented_syl_pos==first_syl_in_next_phrase_pos)?0:std::distance(syl_pos,accented_syl_pos);
        return result;
      }
    };

    struct hts_syl_vowel: public feature_function
    {
      hts_syl_vowel():
        feature_function("syl_vowel")
      {
      }

      value eval(const item& seg) const
      {
        return (is_silence(seg)?x:seg.eval("R:SylStructure.parent.syl_vowel"));
      }
    };

    struct hts_next_syl_vowel: public feature_function
    {
      hts_next_syl_vowel():
        feature_function("next_syl_vowel")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval("R:SylStructure.parent.R:Syllable.n.syl_vowel",x);
      }
    };

    struct hts_next_next_syl_vowel: public feature_function
    {
      hts_next_next_syl_vowel():
        feature_function("next_next_syl_vowel")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval("R:SylStructure.parent.R:Syllable.n.n.syl_vowel",x);
      }
    };

    struct hts_prev_syl_vowel: public feature_function
    {
      hts_prev_syl_vowel():
        feature_function("prev_syl_vowel")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval("R:SylStructure.parent.R:Syllable.p.syl_vowel",x);
      }
    };

    struct hts_prev_prev_syl_vowel: public feature_function
    {
      hts_prev_prev_syl_vowel():
        feature_function("prev_prev_syl_vowel")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval("R:SylStructure.parent.R:Syllable.p.p.syl_vowel",x);
      }
    };

    struct hts_next_syl_vowel_in_word: public feature_function
    {
      hts_next_syl_vowel_in_word():
        feature_function("next_syl_vowel_in_word")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval("R:SylStructure.parent.n.syl_vowel",x);
      }
    };

    struct hts_next_next_syl_vowel_in_word: public feature_function
    {
      hts_next_next_syl_vowel_in_word():
        feature_function("next_next_syl_vowel_in_word")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval("R:SylStructure.parent.n.n.syl_vowel",x);
      }
    };

    struct hts_prev_syl_vowel_in_word: public feature_function
    {
      hts_prev_syl_vowel_in_word():
        feature_function("prev_syl_vowel_in_word")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval("R:SylStructure.parent.p.syl_vowel",x);
      }
    };

    struct hts_prev_prev_syl_vowel_in_word: public feature_function
    {
      hts_prev_prev_syl_vowel_in_word():
        feature_function("prev_prev_syl_vowel_in_word")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval("R:SylStructure.parent.p.p.syl_vowel",x);
      }
    };

    struct hts_first_syl_vowel_in_word: public feature_function
    {
      hts_first_syl_vowel_in_word():
        feature_function("first_syl_vowel_in_word")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval("R:SylStructure.parent.parent.daughter1.syl_vowel",x);
      }
    };

    struct hts_last_syl_vowel_in_word: public feature_function
    {
      hts_last_syl_vowel_in_word():
        feature_function("last_syl_vowel_in_word")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval("R:SylStructure.parent.parent.daughtern.syl_vowel",x);
      }
    };

    struct hts_next_syl_stress: public feature_function
    {
      hts_next_syl_stress():
        feature_function("next_syl_stress")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval(is_silence(seg)?"n.R:SylStructure.parent.stress":"R:SylStructure.parent.R:Syllable.n.stress",zero);
      }
    };

    struct hts_next_syl_accented: public feature_function
    {
      hts_next_syl_accented():
        feature_function("next_syl_accented")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval(is_silence(seg)?"n.R:SylStructure.parent.accented":"R:SylStructure.parent.R:Syllable.n.accented",zero);
      }
    };

    struct hts_next_syl_length: public feature_function
    {
      hts_next_syl_length():
        feature_function("next_syl_length")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval(is_silence(seg)?"n.R:SylStructure.parent.syl_numphones":"R:SylStructure.parent.R:Syllable.n.syl_numphones",zero);
      }
    };

    struct hts_next_syl_coda_length: public feature_function
    {
      hts_next_syl_coda_length():
        feature_function("next_syl_coda_length")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval(is_silence(seg)?"n.R:SylStructure.parent.syl_codasize":"R:SylStructure.parent.R:Syllable.n.syl_codasize",zero);
      }
    };

    struct hts_next_syl_break: public feature_function
    {
      hts_next_syl_break():
        feature_function("next_syl_break")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval(is_silence(seg)?"n.R:SylStructure.parent.syl_break":"R:SylStructure.parent.R:Syllable.n.syl_break",zero);
      }
    };

    struct hts_next_next_syl_lex_tone: public feature_function
    {
      hts_next_next_syl_lex_tone():
        feature_function("next_next_syl_lex_tone")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval(is_silence(seg)?"n.R:SylStructure.parent.R:PhraseSylStructure.n.lex_tone":"R:SylStructure.parent.R:PhraseSylStructure.n.n.lex_tone",x);
      }
    };

    struct hts_next_syl_lex_tone: public feature_function
    {
      hts_next_syl_lex_tone():
        feature_function("next_syl_lex_tone")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval(is_silence(seg)?"n.R:SylStructure.parent.lex_tone":"R:SylStructure.parent.R:PhraseSylStructure.n.lex_tone",x);
      }
    };

    struct hts_prev_word_gpos: public feature_function
    {
      hts_prev_word_gpos():
        feature_function("prev_word_gpos")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval(is_silence(seg)?"p.R:SylStructure.parent.parent.gpos":"R:SylStructure.parent.parent.R:Word.p.gpos",zero);
      }
    };

    struct hts_prev_word_clitic: public feature_function
    {
      hts_prev_word_clitic():
        feature_function("prev_word_clitic")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval(is_silence(seg)?"p.R:SylStructure.parent.parent.clitic":"R:SylStructure.parent.parent.R:Word.p.clitic",zero);
      }
    };

    struct hts_num_syls_in_prev_word: public feature_function
    {
      hts_num_syls_in_prev_word():
        feature_function("num_syls_in_prev_word")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval(is_silence(seg)?"p.R:SylStructure.parent.parent.word_numsyls":"R:SylStructure.parent.parent.R:Word.p.word_numsyls",zero);
      }
    };

    struct hts_word_gpos: public feature_function
    {
      hts_word_gpos():
        feature_function("word_gpos")
      {
      }

      value eval(const item& seg) const
      {
        return (is_silence(seg)?x:seg.eval("R:SylStructure.parent.parent.gpos"));
      }
    };

    struct hts_word_clitic: public feature_function
    {
      hts_word_clitic():
        feature_function("word_clitic")
      {
      }

      value eval(const item& seg) const
      {
        return (is_silence(seg)?x:seg.eval("R:SylStructure.parent.parent.clitic"));
      }
    };

    struct hts_word_break: public feature_function
    {
      hts_word_break():
        feature_function("word_break")
      {
      }

      value eval(const item& seg) const
      {
        return (is_silence(seg)?x:seg.eval("R:SylStructure.parent.parent.word_break"));
      }
    };

    struct hts_num_syls_in_word: public feature_function
    {
      hts_num_syls_in_word():
        feature_function("num_syls_in_word")
      {
      }

      value eval(const item& seg) const
      {
        return (is_silence(seg)?x:seg.eval("R:SylStructure.parent.parent.word_numsyls"));
      }
    };

    struct hts_word_pos_in_phrase_fw: feature_function
    {
      hts_word_pos_in_phrase_fw():
        feature_function("word_pos_in_phrase_fw")
      {
      }

      value eval(const item& seg) const
      {
        return (is_silence(seg)?x:(seg.eval("R:SylStructure.parent.parent.pos_in_phrase").as<unsigned int>()+1));
      }
    };

    struct hts_word_pos_in_phrase_bw: feature_function
    {
      hts_word_pos_in_phrase_bw():
        feature_function("word_pos_in_phrase_bw")
      {
      }

      value eval(const item& seg) const
      {
        return (is_silence(seg)?x:(seg.eval("R:SylStructure.parent.parent.words_out").as<unsigned int>()+1));
      }
    };

    struct hts_num_content_words_in_phrase_before_this_word: public feature_function
    {
      hts_num_content_words_in_phrase_before_this_word():
        feature_function("num_content_words_in_phrase_before_this_word")
      {
      }

      value eval(const item& seg) const
      {
        return (is_silence(seg)?x:(seg.eval("R:SylStructure.parent.parent.content_words_in").as<unsigned int>()+1));
      }
    };

    struct hts_num_content_words_in_phrase_after_this_word: public feature_function
    {
      hts_num_content_words_in_phrase_after_this_word():
        feature_function("num_content_words_in_phrase_after_this_word")
      {
      }

      value eval(const item& seg) const
      {
        return (is_silence(seg)?x:seg.eval("R:SylStructure.parent.parent.content_words_out"));
      }
    };

    struct hts_dist_to_prev_content_word_in_phrase: public feature_function
    {
      hts_dist_to_prev_content_word_in_phrase():
        feature_function("dist_to_prev_content_word_in_phrase")
      {
      }

      value eval(const item& seg) const
      {
        if(is_silence(seg))
          return x;
        const item& word=seg.as("SylStructure").parent().parent().as("Phrase");
        const item& phrase=word.parent();
        item::const_reverse_iterator pos=std::find_if(word.get_reverse_iterator(),phrase.rend(),feature_equals<std::string>("gpos","content"));
        unsigned int result=(pos==phrase.rend())?0:(std::distance(word.get_reverse_iterator(),pos)+1);
        return result;
      }
    };

    struct hts_dist_to_next_content_word_in_phrase: public feature_function
    {
      hts_dist_to_next_content_word_in_phrase():
        feature_function("dist_to_next_content_word_in_phrase")
      {
      }

      value eval(const item& seg) const
      {
        if(is_silence(seg))
          return x;
        const item& word=seg.as("SylStructure").parent().parent().as("Phrase");
        const item& phrase=word.parent();
        item::const_iterator pos=std::find_if(++(word.get_iterator()),phrase.end(),feature_equals<std::string>("gpos","content"));
        unsigned int result=(pos==phrase.end())?0:std::distance(word.get_iterator(),pos);
        return result;
      }
    };

    struct hts_next_word_gpos: public feature_function
    {
      hts_next_word_gpos():
        feature_function("next_word_gpos")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval(is_silence(seg)?"n.R:SylStructure.parent.parent.gpos":"R:SylStructure.parent.parent.R:Word.n.gpos",zero);
      }
    };

    struct hts_next_word_clitic: public feature_function
    {
      hts_next_word_clitic():
        feature_function("next_word_clitic")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval(is_silence(seg)?"n.R:SylStructure.parent.parent.clitic":"R:SylStructure.parent.parent.R:Word.n.clitic",zero);
      }
    };

    struct hts_next_word_break: public feature_function
    {
      hts_next_word_break():
        feature_function("next_word_break")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval(is_silence(seg)?"n.R:SylStructure.parent.parent.word_break":"R:SylStructure.parent.parent.R:Word.n.word_break",zero);
      }
    };

    struct hts_num_syls_in_next_word: public feature_function
    {
      hts_num_syls_in_next_word():
        feature_function("num_syls_in_next_word")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval(is_silence(seg)?"n.R:SylStructure.parent.parent.word_numsyls":"R:SylStructure.parent.parent.R:Word.n.word_numsyls",zero);
      }
    };

    struct hts_num_syls_in_prev_phrase: public feature_function
    {
      hts_num_syls_in_prev_phrase():
        feature_function("num_syls_in_prev_phrase")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval(is_silence(seg)?"p.R:SylStructure.parent.parent.R:Phrase.parent.phrase_numsyls":"R:SylStructure.parent.parent.R:Phrase.parent.p.phrase_numsyls",zero);
      }
    };

    struct hts_num_words_in_prev_phrase: public feature_function
    {
      hts_num_words_in_prev_phrase():
        feature_function("num_words_in_prev_phrase")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval(is_silence(seg)?"p.R:SylStructure.parent.parent.R:Phrase.parent.phrase_numwords":"R:SylStructure.parent.parent.R:Phrase.parent.p.phrase_numwords",zero);
      }
    };

    struct hts_num_syls_in_phrase: public feature_function
    {
      hts_num_syls_in_phrase():
        feature_function("num_syls_in_phrase")
      {
      }

      value eval(const item& seg) const
      {
        return (is_silence(seg)?x:seg.eval("R:SylStructure.parent.parent.R:Phrase.parent.phrase_numsyls"));
      }
    };

    struct hts_num_words_in_phrase: public feature_function
    {
      hts_num_words_in_phrase():
        feature_function("num_words_in_phrase")
      {
      }

      value eval(const item& seg) const
      {
        return (is_silence(seg)?x:seg.eval("R:SylStructure.parent.parent.R:Phrase.parent.phrase_numwords"));
      }
    };

    struct hts_phrase_pos_in_utt_fw: public feature_function
    {
      hts_phrase_pos_in_utt_fw():
        feature_function("phrase_pos_in_utt_fw")
      {
      }

      value eval(const item& seg) const
      {
        if(is_silence(seg))
          return x;
        const item& phrase=seg.as("SylStructure").parent().parent().as("Phrase").parent();
        unsigned int result=std::distance(phrase.get_relation().begin(),phrase.get_iterator())+1;
        return result;
      }
    };

    struct hts_phrase_pos_in_utt_bw: public feature_function
    {
      hts_phrase_pos_in_utt_bw():
        feature_function("phrase_pos_in_utt_bw")
      {
      }

      value eval(const item& seg) const
      {
        if(is_silence(seg))
          return x;
        const item& phrase=seg.as("SylStructure").parent().parent().as("Phrase").parent();
        unsigned int result=std::distance(phrase.get_iterator(),phrase.get_relation().end());
        return result;
      }
    };

    struct hts_phrase_end_tone: public feature_function
    {
      hts_phrase_end_tone():
        feature_function("phrase_end_tone")
      {
      }

      value eval(const item& seg) const
      {
        return (is_silence(seg)?zero:seg.eval("R:SylStructure.parent.parent.R:Phrase.parent.daughtern.R:SylStructure.daughtern.endtone",std::string("NONE")));
      }
    };

    struct hts_num_syls_in_next_phrase: public feature_function
    {
      hts_num_syls_in_next_phrase():
        feature_function("num_syls_in_next_phrase")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval(is_silence(seg)?"n.R:SylStructure.parent.parent.R:Phrase.parent.phrase_numsyls":"R:SylStructure.parent.parent.R:Phrase.parent.n.phrase_numsyls",zero);
      }
    };

    struct hts_num_words_in_next_phrase: public feature_function
    {
      hts_num_words_in_next_phrase():
        feature_function("num_words_in_next_phrase")
      {
      }

      value eval(const item& seg) const
      {
        return seg.eval(is_silence(seg)?"n.R:SylStructure.parent.parent.R:Phrase.parent.phrase_numwords":"R:SylStructure.parent.parent.R:Phrase.parent.n.phrase_numwords",zero);
      }
    };

    struct hts_num_syls_in_utt: public feature_function
    {
      hts_num_syls_in_utt():
        feature_function("num_syls_in_utt")
      {
      }

      value eval(const item& seg) const
      {
        const relation& syl_rel=seg.get_relation().get_utterance().get_relation("Syllable");
        unsigned int result=std::distance(syl_rel.begin(),syl_rel.end());
        return result;
      }
    };

    struct hts_num_words_in_utt: public feature_function
    {
      hts_num_words_in_utt():
        feature_function("num_words_in_utt")
      {
      }

      value eval(const item& seg) const
      {
        const relation& word_rel=seg.get_relation().get_utterance().get_relation("Word");
        unsigned int result=std::distance(word_rel.begin(),word_rel.end());
        return result;
      }
    };

    struct hts_num_phrases_in_utt: public feature_function
    {
      hts_num_phrases_in_utt():
        feature_function("num_phrases_in_utt")
      {
      }

      value eval(const item& seg) const
      {
        const relation& phrase_rel=seg.get_relation().get_utterance().get_relation("Phrase");
        unsigned int result=std::distance(phrase_rel.begin(),phrase_rel.end());
        return result;
      }
    };

    struct hts_pos_in_word_fw: public feature_function
    {
      hts_pos_in_word_fw():
        feature_function("pos_in_word_fw")
      {
      }

      value eval(const item& seg) const
      {
        if(is_silence(seg))
          return x;
        const item& seg_in_word=seg.as("Transcription");
        const item& word=seg_in_word.parent();
        unsigned int result=std::distance(word.begin(),seg_in_word.get_iterator())+1;
        return result;
      }
    };

    struct hts_pos_in_word_bw: public feature_function
    {
      hts_pos_in_word_bw():
        feature_function("pos_in_word_bw")
      {
      }

      value eval(const item& seg) const
      {
        if(is_silence(seg))
          return x;
        const item& seg_in_word=seg.as("Transcription");
        const item& word=seg_in_word.parent();
        unsigned int result=std::distance(seg_in_word.get_iterator(),word.end());
        return result;
      }
    };

    struct hts_dist_to_prev_stressed_syl_in_word: public feature_function
    {
      hts_dist_to_prev_stressed_syl_in_word():
        feature_function("dist_to_prev_stressed_syl_in_word")
      {
      }

      value eval(const item& seg) const
      {
        if(is_silence(seg))
          return x;
        const item& syl=seg.as("SylStructure").parent();
        const item& word=syl.parent();
        item::const_reverse_iterator syl_pos=syl.get_reverse_iterator();
        item::const_reverse_iterator stressed_syl_pos=std::find_if(syl_pos,word.rend(),feature_equals<std::string>("stress","1"));
        unsigned int result=(stressed_syl_pos==word.rend())?0:(std::distance(syl_pos,stressed_syl_pos)+1);
        return result;
      }
    };

    struct hts_dist_to_next_stressed_syl_in_word: public feature_function
    {
      hts_dist_to_next_stressed_syl_in_word():
        feature_function("dist_to_next_stressed_syl_in_word")
      {
      }

      value eval(const item& seg) const
      {
        if(is_silence(seg))
          return x;
        const item& syl=seg.as("SylStructure").parent();
        const item& word=syl.parent();
        item::const_iterator syl_pos=syl.get_iterator();
        item::const_iterator stressed_syl_pos=std::find_if(++item::const_iterator(syl_pos),word.end(),feature_equals<std::string>("stress","1"));
        unsigned int result=(stressed_syl_pos==word.end())?0:std::distance(syl_pos,stressed_syl_pos);
        return result;
      }
    };

    struct hts_utt_is_question: public feature_function
    {
      hts_utt_is_question():
        feature_function("utt_is_question")
      {
      }

        value eval(const item& seg) const
      {
        const utterance& utt=seg.get_relation().get_utterance();
        const item& word=utt.get_relation("Word").last().as("TokStructure");
        const item& token=word.parent();
        const item& parent_token=token.parent();
        std::string result("0");
        if(std::find_if(++(token.get_iterator()),parent_token.end(),feature_equals<std::string>("name","\?"))!=parent_token.end())
          result="1";
        return result;
      }
    };

    struct hts_utt_type: public feature_function
    {
      hts_utt_type():
        feature_function("utt_type")
      {
      }

        value eval(const item& seg) const
      {
        const utterance& utt=seg.get_relation().get_utterance();
        std::string type=utt.get_utt_type();
        if(utt.has_voice()&&!utt.get_voice().get_info().supports_utt_type(type))
          type="s";
        return type;
      }
    };

    struct hts_num_consonants_to_end_of_cluster: public feature_function
    {
      hts_num_consonants_to_end_of_cluster():
        feature_function("num_consonants_to_end_of_cluster")
      {
      }

        value eval(const item& seg) const
      {
        if(is_silence(seg))
          return x;
        unsigned int count=0;
        item::const_iterator it=seg.as("Transcription").get_iterator();
        while(it->has_next())
          {
            ++it;
            if(it->eval("ph_vc").as<std::string>()!="-")
              break;
            ++count;
          }
        return count;
      }
    };

    struct hts_num_consonants_to_start_of_cluster: public feature_function
    {
      hts_num_consonants_to_start_of_cluster():
        feature_function("num_consonants_to_start_of_cluster")
      {
      }

        value eval(const item& seg) const
      {
        if(is_silence(seg))
          return x;
        unsigned int count=0;
        item::const_iterator it=seg.as("Transcription").get_iterator();
        while(it->has_prev())
          {
            --it;
            if(it->eval("ph_vc").as<std::string>()!="-")
              break;
            ++count;
          }
        return count;
      }
    };

    struct hts_num_consonants_in_cluster: public feature_function
    {
      hts_num_consonants_in_cluster():
        feature_function("num_consonants_in_cluster")
      {
      }

        value eval(const item& seg) const
      {
        if(seg.eval("ph_vc").as<std::string>()!="-")
          return x;
        unsigned int count=1;
        item::const_iterator it1=seg.as("Transcription").get_iterator();
        item::const_iterator it2=it1;
        while(it1->has_next())
          {
            ++it1;
            if(it1->eval("ph_vc").as<std::string>()!="-")
              break;
            ++count;
          }
        while(it2->has_prev())
          {
            --it2;
            if(it2->eval("ph_vc").as<std::string>()!="-")
              break;
            ++count;
          }
        return count;
      }
    };

    struct hts_num_consonants_to_next_vowel: public feature_function
    {
      hts_num_consonants_to_next_vowel():
        feature_function("num_consonants_to_next_vowel")
      {
      }

        value eval(const item& seg) const
      {
        if(is_silence(seg))
          return x;
        unsigned int count=0;
        item::const_iterator it=seg.get_iterator();
        while(it->has_next())
          {
            ++it;
            if(it->eval("ph_vc").as<std::string>()!="-")
              break;
            ++count;
          }
        return count;
      }
    };

    struct hts_num_consonants_to_prev_vowel: public feature_function
    {
      hts_num_consonants_to_prev_vowel():
        feature_function("num_consonants_to_prev_vowel")
      {
      }

        value eval(const item& seg) const
      {
        if(is_silence(seg))
          return x;
        unsigned int count=0;
        item::const_iterator it=seg.get_iterator();
        while(it->has_prev())
          {
            --it;
            if(it->eval("ph_vc").as<std::string>()!="-")
              break;
            ++count;
          }
        return count;
      }
    };

    struct hts_num_intervocalic_consonants: public feature_function
    {
      hts_num_intervocalic_consonants():
        feature_function("num_intervocalic_consonants")
      {
      }

        value eval(const item& seg) const
      {
        if(seg.eval("ph_vc").as<std::string>()!="-")
          return x;
        unsigned int count=1;
        item::const_iterator it1=seg.get_iterator();
        item::const_iterator it2=it1;
        while(it1->has_next())
          {
            ++it1;
            if(it1->eval("ph_vc").as<std::string>()!="-")
              break;
            ++count;
          }
        while(it2->has_prev())
          {
            --it2;
            if(it2->eval("ph_vc").as<std::string>()!="-")
              break;
            ++count;
          }
        return count;
      }
    };

    struct hts_num_vowels_to_start_of_word: public feature_function
    {
      hts_num_vowels_to_start_of_word():
        feature_function("num_vowels_to_start_of_word")
      {
      }

        value eval(const item& seg) const
      {
        if(is_silence(seg))
          return x;
        const item& s=seg.as("Transcription");
        item::const_iterator it1=s.parent().begin();
        item::const_iterator it2=s.get_iterator();
        unsigned int count=std::count_if(it1,it2,feature_equals<std::string>("ph_vc","+"));
        return count;
      }
    };

    struct hts_num_vowels_to_end_of_word: public feature_function
    {
      hts_num_vowels_to_end_of_word():
        feature_function("num_vowels_to_end_of_word")
      {
      }

        value eval(const item& seg) const
      {
        if(is_silence(seg))
          return x;
        const item& s=seg.as("Transcription");
        item::const_iterator it2=s.parent().end();
        item::const_iterator it1=++(s.get_iterator());
        unsigned int count=std::count_if(it1,it2,feature_equals<std::string>("ph_vc","+"));
        return count;
      }
    };

    struct hts_num_vowels_to_start_of_phrase: public feature_function
    {
      hts_num_vowels_to_start_of_phrase():
        feature_function("num_vowels_to_start_of_phrase")
      {
      }

        value eval(const item& seg) const
      {
        if(is_silence(seg))
          return x;
        item::const_iterator it1=seg.as("Transcription").parent().as("Phrase").parent().first_child().as("Transcription").first_child().as("Segment").get_iterator();
        item::const_iterator it2=seg.get_iterator();
        unsigned int count=std::count_if(it1,it2,feature_equals<std::string>("ph_vc","+"));
        return count;
      }
    };

    struct hts_num_vowels_to_end_of_phrase: public feature_function
    {
      hts_num_vowels_to_end_of_phrase():
        feature_function("num_vowels_to_end_of_phrase")
      {
      }

        value eval(const item& seg) const
      {
        if(is_silence(seg))
          return x;
        item::const_iterator it1=++(seg.get_iterator());
        item::const_iterator it2=++(seg.as("Transcription").parent().as("Phrase").parent().last_child().as("Transcription").last_child().as("Segment").get_iterator());
        unsigned int count=std::count_if(it1,it2,feature_equals<std::string>("ph_vc","+"));
        return count;
      }
    };

    struct hts_syl_part: public feature_function
    {
      hts_syl_part():
        feature_function("syl_part")
      {
      }

      value eval(const item& seg) const
      {
        if(is_silence(seg))
          return x;
        if(seg.eval("ph_vc").as<std::string>()=="+")
          return std::string("rime");
        const item& sseg=seg.as("SylStructure");
        const item& syl=sseg.parent();
        item::const_iterator seg_pos=sseg.get_iterator();
        for(item::const_iterator pos=syl.begin();pos!=seg_pos;++pos)
          {
            if(pos->eval("ph_vc").as<std::string>()=="+")
              return std::string("rime");
          }
        return std::string("onset");
      }
    };

    struct hts_first_gpos_in_phrase: public feature_function
    {
      hts_first_gpos_in_phrase():
        feature_function("first_gpos_in_phrase")
      {
      }

      value eval(const item& seg) const
      {
        return is_silence(seg)?x:seg.eval("R:SylStructure.parent.parent.R:Phrase.parent.daughter1.gpos",zero);
      }
    };

    struct hts_last_gpos_in_phrase: public feature_function
    {
      hts_last_gpos_in_phrase():
        feature_function("last_gpos_in_phrase")
      {
      }

      value eval(const item& seg) const
      {
        return is_silence(seg)?x:seg.eval("R:SylStructure.parent.parent.R:Phrase.parent.daughtern.gpos",zero);
      }
    };

    struct hts_ext_phon_feat: public feature_function
    {
    private:
      const std::string full_name;

    public:
      hts_ext_phon_feat(const std::string& hts_prefix,const std::string& path,const std::string& short_name):
        feature_function(hts_prefix+"ext_"+short_name),
        full_name(path+"ph_ext_"+short_name)
      {
      }

      value eval(const item& seg) const
      {
        return is_silence(seg)?x:seg.eval(full_name,zero);
      }
    };

    struct hts_ph_flag_feat: public feature_function
    {
    private:
      const std::string full_name;

    public:
      hts_ph_flag_feat(const std::string& hts_prefix,const std::string& path,const std::string& short_name):
        feature_function(hts_prefix+"ph_flag_"+short_name),
        full_name(path+"ph_flag_"+short_name)
      {
      }

      value eval(const item& seg) const
      {
        return is_silence(seg)?x:seg.eval(full_name,zero);
      }
    };

    struct hts_dist_to_prev_round_vowel_in_word: public feature_function
    {
      hts_dist_to_prev_round_vowel_in_word():
        feature_function("dist_to_prev_round_vowel_in_word")
      {
      }

      value eval(const item& seg) const
      {
        if(is_silence(seg))
          return x;
        const item& syl=seg.as("SylStructure").parent();
        const item& word=syl.parent();
        item::const_reverse_iterator syl_pos=syl.get_reverse_iterator();
        item::const_reverse_iterator round_syl_pos=std::find_if(syl_pos,word.rend(),feature_equals<std::string>("syl_vowel_vrnd","+"));
        unsigned int result=(round_syl_pos==word.rend())?0:(std::distance(syl_pos,round_syl_pos)+1);
        return result;
      }
    };

  }

    struct hts_syl_vowel_ph_flag_feat: public feature_function
    {
    private:
      const std::string full_name;

    public:
      hts_syl_vowel_ph_flag_feat(const std::string& hts_prefix,const std::string& path,const std::string& short_name):
        feature_function(hts_prefix+"syl_vowel_ph_flag_"+short_name),
        full_name("R:SylStructure.parent."+path+"syl_vowel_ph_flag_"+short_name)
      {
      }

      value eval(const item& seg) const
      {
        return is_silence(seg)?x:seg.eval(full_name,zero);
      }
    };

  void hts_labeller::load_label_format_description(const std::string& file_path)
  {
    std::ifstream infile;
    io::open_ifstream(infile,file_path);
    std::vector<char> text;
    std::copy(std::istreambuf_iterator<char>(infile),std::istreambuf_iterator<char>(),std::back_inserter(text));
    text.push_back('\0');
    rapidxml::xml_document<> doc;
    doc.parse<rapidxml::parse_trim_whitespace>(&text[0]);
    std::string err_msg("Hts label format description is incorrect");
    rapidxml::xml_node<>* child_node=doc.first_node("labelling");
    if((child_node==0)||(child_node->type()!=rapidxml::node_element))
      throw file_format_error(err_msg);
    child_node=child_node->first_node("format");
    if((child_node==0)||(child_node->type()!=rapidxml::node_element))
      throw file_format_error(err_msg);
    hts_feature feat;
    rapidxml::node_iterator<char> end;
    rapidxml::xml_attribute<>* name_attr=0;
    for(rapidxml::node_iterator<char> it(child_node);it!=end;++it)
      {
        switch(it->type())
          {
          case rapidxml::node_data:
            feat.prefix.assign(it->value(),it->value_size());
            break;
          case rapidxml::node_element:
            name_attr=it->first_attribute("name");
            if(name_attr==0)
              throw file_format_error(err_msg);
            feat.name.assign(name_attr->value(),name_attr->value_size());
            features.push_back(feat);
            feat.prefix.clear();
            feat.name.clear();
            break;
          default:
            break;
          }
      }
  }

  std::string hts_labeller::eval_segment_label(const item& seg) const
  {
    std::ostringstream s;
    s.imbue(std::locale::classic());
    for(std::vector<hts_feature>::const_iterator it(features.begin());it!=features.end();++it)
      {
        if(! it->function)
          throw hts_feature_undefined(it->name);
        s << it->prefix;
        s << it->function->eval(seg);
      }
    return s.str();
  }

  void hts_labeller::define_feature(const std::shared_ptr<feature_function>& f)
  {
    std::string name(f->get_name());
    for(std::vector<hts_feature>::iterator it(features.begin());it!=features.end();++it)
      {
        if(it->name==name)
          it->function=f;
      }
  }

  void hts_labeller::define_default_features()
  {
    define_feature(std::shared_ptr<feature_function>(new hts_none));
    define_feature(std::shared_ptr<feature_function>(new hts_prev_prev_name));
    define_feature(std::shared_ptr<feature_function>(new hts_prev_name));
    define_feature(std::shared_ptr<feature_function>(new hts_name));
    define_feature(std::shared_ptr<feature_function>(new hts_next_name));
    define_feature(std::shared_ptr<feature_function>(new hts_next_next_name));
    define_feature(std::shared_ptr<feature_function>(new hts_pos_in_syl_fw));
    define_feature(std::shared_ptr<feature_function>(new hts_pos_in_syl_bw));
    define_feature(std::shared_ptr<feature_function>(new hts_prev_syl_stress));
    define_feature(std::shared_ptr<feature_function>(new hts_prev_syl_accented));
    define_feature(std::shared_ptr<feature_function>(new hts_prev_syl_lex_tone));
    define_feature(std::shared_ptr<feature_function>(new hts_prev_prev_syl_lex_tone));
    define_feature(std::shared_ptr<feature_function>(new hts_prev_syl_length));
    define_feature(std::shared_ptr<feature_function>(new hts_syl_stress));
    define_feature(std::shared_ptr<feature_function>(new hts_syl_accented));
    define_feature(std::shared_ptr<feature_function>(new hts_syl_lex_tone));
    define_feature(std::shared_ptr<feature_function>(new hts_syl_length));
    define_feature(std::shared_ptr<feature_function>(new hts_syl_break));
    define_feature(std::shared_ptr<feature_function>(new hts_syl_pos_in_word_fw));
    define_feature(std::shared_ptr<feature_function>(new hts_syl_pos_in_word_bw));
define_feature(std::shared_ptr<feature_function>(new hts_syl_pos_type));
    define_feature(std::shared_ptr<feature_function>(new hts_syl_pos_in_phrase_fw));
    define_feature(std::shared_ptr<feature_function>(new hts_rel_syl_in));
    define_feature(std::shared_ptr<feature_function>(new hts_rel_syl_out));
    define_feature(std::shared_ptr<feature_function>(new hts_syl_pos_in_phrase_bw));
    define_feature(std::shared_ptr<feature_function>(new hts_num_stressed_syls_in_phrase_before_this_syl));
    define_feature(std::shared_ptr<feature_function>(new hts_num_stressed_syls_in_phrase_after_this_syl));
    define_feature(std::shared_ptr<feature_function>(new hts_num_accented_syls_in_phrase_before_this_syl));
    define_feature(std::shared_ptr<feature_function>(new hts_num_accented_syls_in_phrase_after_this_syl));
    define_feature(std::shared_ptr<feature_function>(new hts_dist_to_prev_stressed_syl_in_phrase));
    define_feature(std::shared_ptr<feature_function>(new hts_dist_to_next_stressed_syl_in_phrase));
    define_feature(std::shared_ptr<feature_function>(new hts_dist_to_prev_accented_syl_in_phrase));
    define_feature(std::shared_ptr<feature_function>(new hts_dist_to_next_accented_syl_in_phrase));
    define_feature(std::shared_ptr<feature_function>(new hts_syl_vowel));
    define_feature(std::shared_ptr<feature_function>(new hts_next_syl_vowel));
    define_feature(std::shared_ptr<feature_function>(new hts_next_next_syl_vowel));
    define_feature(std::shared_ptr<feature_function>(new hts_prev_syl_vowel));
    define_feature(std::shared_ptr<feature_function>(new hts_prev_prev_syl_vowel));
    define_feature(std::shared_ptr<feature_function>(new hts_next_syl_vowel_in_word));
    define_feature(std::shared_ptr<feature_function>(new hts_next_next_syl_vowel_in_word));
    define_feature(std::shared_ptr<feature_function>(new hts_prev_syl_vowel_in_word));
    define_feature(std::shared_ptr<feature_function>(new hts_prev_prev_syl_vowel_in_word));
    define_feature(std::shared_ptr<feature_function>(new hts_first_syl_vowel_in_word));
    define_feature(std::shared_ptr<feature_function>(new hts_last_syl_vowel_in_word));
    define_feature(std::shared_ptr<feature_function>(new hts_next_syl_stress));
    define_feature(std::shared_ptr<feature_function>(new hts_next_syl_accented));
    define_feature(std::shared_ptr<feature_function>(new hts_next_syl_lex_tone));
    define_feature(std::shared_ptr<feature_function>(new hts_next_next_syl_lex_tone));
    define_feature(std::shared_ptr<feature_function>(new hts_next_syl_length));
        define_feature(std::shared_ptr<feature_function>(new hts_next_syl_break));
    define_feature(std::shared_ptr<feature_function>(new hts_prev_word_gpos));
    define_feature(std::shared_ptr<feature_function>(new hts_prev_word_clitic));
    define_feature(std::shared_ptr<feature_function>(new hts_num_syls_in_prev_word));
    define_feature(std::shared_ptr<feature_function>(new hts_word_gpos));
    define_feature(std::shared_ptr<feature_function>(new hts_word_clitic));
    define_feature(std::shared_ptr<feature_function>(new hts_word_break));
    define_feature(std::shared_ptr<feature_function>(new hts_num_syls_in_word));
    define_feature(std::shared_ptr<feature_function>(new hts_word_pos_in_phrase_fw));
    define_feature(std::shared_ptr<feature_function>(new hts_word_pos_in_phrase_bw));
    define_feature(std::shared_ptr<feature_function>(new hts_num_content_words_in_phrase_before_this_word));
    define_feature(std::shared_ptr<feature_function>(new hts_num_content_words_in_phrase_after_this_word));
    define_feature(std::shared_ptr<feature_function>(new hts_dist_to_prev_content_word_in_phrase));
    define_feature(std::shared_ptr<feature_function>(new hts_dist_to_next_content_word_in_phrase));
    define_feature(std::shared_ptr<feature_function>(new hts_next_word_gpos));
    define_feature(std::shared_ptr<feature_function>(new hts_next_word_clitic));
    define_feature(std::shared_ptr<feature_function>(new hts_next_word_break));
    define_feature(std::shared_ptr<feature_function>(new hts_num_syls_in_next_word));
    define_feature(std::shared_ptr<feature_function>(new hts_num_syls_in_prev_phrase));
    define_feature(std::shared_ptr<feature_function>(new hts_num_words_in_prev_phrase));
    define_feature(std::shared_ptr<feature_function>(new hts_num_syls_in_phrase));
    define_feature(std::shared_ptr<feature_function>(new hts_num_words_in_phrase));
    define_feature(std::shared_ptr<feature_function>(new hts_phrase_pos_in_utt_fw));
    define_feature(std::shared_ptr<feature_function>(new hts_phrase_pos_in_utt_bw));
    define_feature(std::shared_ptr<feature_function>(new hts_phrase_end_tone));
    define_feature(std::shared_ptr<feature_function>(new hts_num_syls_in_next_phrase));
    define_feature(std::shared_ptr<feature_function>(new hts_num_words_in_next_phrase));
    define_feature(std::shared_ptr<feature_function>(new hts_num_syls_in_utt));
    define_feature(std::shared_ptr<feature_function>(new hts_num_words_in_utt));
    define_feature(std::shared_ptr<feature_function>(new hts_num_phrases_in_utt));
    define_feature(std::shared_ptr<feature_function>(new hts_pos_in_word_fw));
    define_feature(std::shared_ptr<feature_function>(new hts_pos_in_word_bw));
    define_feature(std::shared_ptr<feature_function>(new hts_dist_to_prev_stressed_syl_in_word));
    define_feature(std::shared_ptr<feature_function>(new hts_dist_to_next_stressed_syl_in_word));
    define_feature(std::shared_ptr<feature_function>(new hts_utt_is_question));
define_feature(std::shared_ptr<feature_function>(new hts_utt_type));
    define_feature(std::shared_ptr<feature_function>(new hts_num_consonants_to_end_of_cluster));
    define_feature(std::shared_ptr<feature_function>(new hts_num_consonants_to_start_of_cluster));
    define_feature(std::shared_ptr<feature_function>(new hts_num_consonants_to_next_vowel));
    define_feature(std::shared_ptr<feature_function>(new hts_num_consonants_to_prev_vowel));
    define_feature(std::shared_ptr<feature_function>(new hts_num_consonants_in_cluster));
    define_feature(std::shared_ptr<feature_function>(new hts_num_intervocalic_consonants));
    define_feature(std::shared_ptr<feature_function>(new hts_num_vowels_to_start_of_word));
    define_feature(std::shared_ptr<feature_function>(new hts_num_vowels_to_end_of_word));
    define_feature(std::shared_ptr<feature_function>(new hts_num_vowels_to_start_of_phrase));
    define_feature(std::shared_ptr<feature_function>(new hts_num_vowels_to_end_of_phrase));
    define_feature(std::shared_ptr<feature_function>(new hts_syl_part));
    define_feature(std::shared_ptr<feature_function>(new hts_first_gpos_in_phrase));
    define_feature(std::shared_ptr<feature_function>(new hts_last_gpos_in_phrase));
define_feature(std::shared_ptr<feature_function>(new hts_prev_syl_coda_length));
define_feature(std::shared_ptr<feature_function>(new hts_syl_coda_length));
define_feature(std::shared_ptr<feature_function>(new hts_next_syl_coda_length));
 define_feature(std::shared_ptr<feature_function>(new hts_dist_to_prev_round_vowel_in_word));
  }

  void hts_labeller::define_extra_phonetic_feature(const std::string& name)
  {
    define_feature(std::shared_ptr<feature_function>(new hts_ext_phon_feat("","",name)));
    define_feature(std::shared_ptr<feature_function>(new hts_ext_phon_feat("next_","n.",name)));
    define_feature(std::shared_ptr<feature_function>(new hts_ext_phon_feat("prev_","p.",name)));
    define_feature(std::shared_ptr<feature_function>(new hts_ext_phon_feat("next_next_","n.n.",name)));
    define_feature(std::shared_ptr<feature_function>(new hts_ext_phon_feat("prev_prev_","p.p.",name)));
}

  void hts_labeller::define_ph_flag_feature(const std::string& name)
  {
    define_feature(std::shared_ptr<feature_function>(new hts_ph_flag_feat("","",name)));
    define_feature(std::shared_ptr<feature_function>(new hts_ph_flag_feat("next_","n.",name)));
    define_feature(std::shared_ptr<feature_function>(new hts_ph_flag_feat("prev_","p.",name)));
    define_feature(std::shared_ptr<feature_function>(new hts_ph_flag_feat("next_next_","n.n.",name)));
    define_feature(std::shared_ptr<feature_function>(new hts_ph_flag_feat("prev_prev_","p.p.",name)));
    define_feature(std::shared_ptr<feature_function>(new hts_syl_vowel_ph_flag_feat("","",name)));
    define_feature(std::shared_ptr<feature_function>(new hts_syl_vowel_ph_flag_feat("next_in_word_","n.",name)));
    define_feature(std::shared_ptr<feature_function>(new hts_syl_vowel_ph_flag_feat("prev_in_word_","p.",name)));
    define_feature(std::shared_ptr<feature_function>(new hts_syl_vowel_ph_flag_feat("next_next_in_word_","n.n.",name)));
    define_feature(std::shared_ptr<feature_function>(new hts_syl_vowel_ph_flag_feat("prev_prev_in_word_","p.p.",name)));
    define_feature(std::shared_ptr<feature_function>(new hts_syl_vowel_ph_flag_feat("first_in_word_","parent.daughter1.",name)));
    define_feature(std::shared_ptr<feature_function>(new hts_syl_vowel_ph_flag_feat("last_in_word_","parent.daughtern.",name)));
    define_feature(std::shared_ptr<feature_function>(new hts_syl_vowel_ph_flag_feat("next_","R:Syllable.n.",name)));
    define_feature(std::shared_ptr<feature_function>(new hts_syl_vowel_ph_flag_feat("prev_","R:Syllable.p.",name)));
    define_feature(std::shared_ptr<feature_function>(new hts_syl_vowel_ph_flag_feat("next_next_","R:Syllable.n.n.",name)));
    define_feature(std::shared_ptr<feature_function>(new hts_syl_vowel_ph_flag_feat("prev_prev_","R:Syllable.p.p.",name)));
}
}
 
