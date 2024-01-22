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

#ifndef RHVOICE_DATA_ONLY_LANGUAGE_HPP
#define RHVOICE_DATA_ONLY_LANGUAGE_HPP

#include "str.hpp"
#include "property.hpp"
#include "fst.hpp"
#include "language.hpp"

namespace RHVoice
{
  class data_only_language_info: public language_info
  {
  public:
    data_only_language_info(const std::string& data_path,const std::string& userdict_path);

    std::string get_country() const;

    #ifdef WIN32
    unsigned short get_id() const
    {
      return loc.ms_id;
    }
    #endif

  private:

struct locale_info
{
  string_property language2{"language2"};
  string_property language3{"language3"};
  string_property country2{"country2"};
  string_property country3{"country3"};
  numeric_property<unsigned int> ms_id{"ms_id", 0, 0, 0x7fff};
};

    std::shared_ptr<language> create_instance() const;
    locale_info loc;
  };

  class data_only_language: public language
  {
  public:
    explicit data_only_language(const data_only_language_info& info);

    const data_only_language_info& get_info() const
    {
      return info;
    }

    std::vector<std::string> get_word_transcription(const item& word) const;
  private:

    void before_g2p(utterance& u) const;

    const data_only_language_info& info;
    const fst g2p_fst;
    const fst lseq_fst;
std::unique_ptr<fst> g2g_fst;
std::unique_ptr<fst> lex_fst;
std::unique_ptr<fst> gg2g_fst;
  };
}
#endif
