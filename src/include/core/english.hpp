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

#ifndef RHVOICE_ENGLISH_HPP
#define RHVOICE_ENGLISH_HPP

#include "str.hpp"
#include "fst.hpp"
#include "dtree.hpp"
#include "lts.hpp"
#include "language.hpp"

namespace RHVoice
{
  class english_info: public language_info
  {
  public:
    english_info(const std::string& data_path,const std::string& userdict_path);

    #ifdef WIN32
    unsigned short get_id() const
    {
      return 0x0409;
    }
    #endif

    std::string get_country() const
    {
      return "USA/US";
    }

  private:
    std::shared_ptr<language> create_instance() const;
  };

  class english: public language
  {
  public:
    explicit english(const english_info& info_);

    const english_info& get_info() const
    {
      return info;
    }

    std::vector<std::string> get_word_transcription(const item& word) const;

  private:
    void decode_as_special_symbol(item& token,const std::string& name,const std::string& type) const;
    void post_lex(utterance& u) const;

    void predict_accents_and_tones(utterance& u) const;
    void correct_pronunciation_of_articles(utterance& u) const;
    void correct_pronunciation_of_contractions(utterance& u) const;

    const english_info& info;
    const fst cmulex_fst;
    const lts cmulex_lts;
    const fst lseq_fst;
    const dtree accents_dtree;
    const dtree tones_dtree;
  };
}
#endif
