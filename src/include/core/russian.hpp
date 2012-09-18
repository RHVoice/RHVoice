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

#ifndef RHVOICE_RUSSIAN_HPP
#define RHVOICE_RUSSIAN_HPP

#include "str.hpp"
#include "fst.hpp"
#include "rules.hpp"
#include "language.hpp"

namespace RHVoice
{
  class russian: public language
  {
  public:
    explicit russian(const language_info& info);

  private:
    void mark_clitics(utterance& u) const;
    void reduce_vowels(utterance& u) const;
    void do_final_devoicing_and_voicing_assimilation(utterance& u) const;
    void rename_unstressed_vowels(utterance& u) const;

    void decode_as_word(item& token) const;
    void transcribe_word(item& word) const;
    void post_lex(utterance& u) const;

    const fst clit_fst;
    const fst g2p_fst;
    const fst lseq_fst;
    const fst untranslit_fst;
    const fst dict_fst;
    const rules<uint8_t> stress_rules;
  };

  class russian_info: public language_info
  {
  public:
    explicit russian_info(const std::string& data_path):
      language_info("Russian",data_path)
    {
      set_alpha2_code("ru");
      set_alpha3_code("rus");
      #ifdef WIN32
      set_id(0x0419);
      #endif
      register_letter_range(0x430,32);
      register_letter_range(0x410,32);
      register_letter(0x451);
      register_letter(0x401);
    }

  private:
    smart_ptr<language> create_instance() const;
  };
}
#endif
