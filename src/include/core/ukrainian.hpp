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

#ifndef RHVOICE_UKRAINIAN_HPP
#define RHVOICE_UKRAINIAN_HPP

#include "str.hpp"
#include "fst.hpp"
#include "language.hpp"
#include "rules.hpp"

namespace RHVoice
{
  class ukrainian_info: public language_info
  {
  public:
    ukrainian_info(const std::string& data_path,const std::string& userdict_path);

    #ifdef WIN32
    unsigned short get_id() const
    {
      return 1058;
    }
    #endif

    std::string get_country() const
    {
      return "UKR/UA";
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

  class ukrainian: public language
  {
  public:
    explicit ukrainian(const ukrainian_info& info_);

    const ukrainian_info& get_info() const
    {
      return info;
    }

    std::vector<std::string> get_word_transcription(const item& word) const;

    void decode_as_word(item& token,const std::string& token_name) const;

  private:
    void post_lex(utterance& u) const;

    const ukrainian_info& info;
    const fst g2p_fst;
    const fst untranslit_fst;
    const fst lseq_fst;
    const fst stress_fst;
    const rules<uint8_t> stress_rules;
    std::unique_ptr<fst> stress_marks_fst;
  };
}
#endif
