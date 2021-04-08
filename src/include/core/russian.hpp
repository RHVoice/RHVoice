/* Copyright (C) 2012, 2014  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_RUSSIAN_HPP
#define RHVOICE_RUSSIAN_HPP

#include <memory>
#include "str.hpp"
#include "fst.hpp"
#include "rules.hpp"
#include "language.hpp"

namespace RHVoice
{
  class russian_info: public language_info
  {
  public:
    russian_info(const std::string& data_path,const std::string& userdict_path);

    #ifdef WIN32
    unsigned short get_id() const
    {
      return 0x0419;
    }
    #endif

    std::string get_country() const
    {
      return "RUS/RU";
    }

    bool supports_stress_marks() const
    {
      return true;
    }

    bool supports_pseudo_english() const
    {
      return true;
    }

  private:
    std::shared_ptr<language> create_instance() const;
  };

  class russian: public language
  {
  public:
    explicit russian(const russian_info& info);

    const russian_info& get_info() const
    {
      return info;
    }

    std::vector<std::string> get_word_transcription(const item& word) const;

  private:
    bool decode_as_russian_word(item& token,const std::string& token_name) const;
    void decode_as_word(item& token,const std::string& token_name) const;

    void mark_clitics(utterance& u) const;
    void reduce_vowels(utterance& u) const;
    void do_final_devoicing_and_voicing_assimilation(utterance& u) const;
    void rename_unstressed_vowels(utterance& u) const;

    bool transcribe_letter_sequence(const item& word,std::vector<std::string>& transcription) const;
    bool transcribe_word_with_stress_marks(const item& word,std::vector<std::string>& transcription) const;
    bool transcribe_word_from_dict(const item& word,std::vector<std::string>& transcription) const;
    bool transcribe_monosyllabic_word(const item& word,std::vector<std::string>& transcription) const;
    bool transcribe_word_from_stress_dict(const item& word,std::vector<std::string>& transcription) const;
    bool transcribe_word_applying_stress_rules(const item& word,std::vector<std::string>& transcription) const;
    bool transcribe_unknown_word(const item& word,std::vector<std::string>& transcription) const;
    bool transcribe_word_from_rulex(const item& word,std::vector<std::string>& transcription) const;

    void post_lex(utterance& u) const;

    const russian_info& info;
    const fst clit_fst;
    const fst g2p_fst;
    const fst lseq_fst;
    const fst untranslit_fst;
    const fst split_fst;
    const fst dict_fst;
    const fst stress_fst;
    const rules<uint8_t> stress_rules;
    std::unique_ptr<fst> rulex_dict_fst,rulex_rules_fst,stress_marks_fst;
  };
}
#endif
