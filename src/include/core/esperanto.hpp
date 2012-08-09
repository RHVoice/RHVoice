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

#ifndef RHVOICE_ESPERANTO_HPP
#define RHVOICE_ESPERANTO_HPP

#include "str.hpp"
#include "fst.hpp"
#include "language.hpp"

namespace RHVoice
{
  class esperanto: public language
  {
  public:
    explicit esperanto(const language_info& info);

  private:
    void decode_as_word(item& token) const;
    void decode_as_letter_sequence(item& token) const;
    void transcribe_word(item& word) const;
    void post_lex(utterance& u) const;

    const fst g2p_fst;
    const fst untranslit_fst;
  };

  class esperanto_info: public language_info
  {
  public:
    explicit esperanto_info(const std::string& data_path):
      language_info("Esperanto",data_path)
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
    }

  private:
    smart_ptr<language> create_instance() const;
  };
}
#endif
