/* Copyright (C) 2017, 2018  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_TATAR_HPP
#define RHVOICE_TATAR_HPP

#include "str.hpp"
#include "fst.hpp"
#include "language.hpp"

namespace RHVoice
{
  class tatar_info: public language_info
  {
  public:
    tatar_info(const std::string& data_path,const std::string& userdict_path);

    #ifdef WIN32
    unsigned short get_id() const
    {
      return 1092;
    }
    #endif

    std::string get_country() const
    {
      return "RUS/RU";
    }

    bool supports_pseudo_english() const
    {
      return true;
    }

  private:
    std::shared_ptr<language> create_instance() const;
  };

  class tatar: public language
  {
  public:
    explicit tatar(const tatar_info& info_);

    const tatar_info& get_info() const
    {
      return info;
    }

    std::vector<std::string> get_word_transcription(const item& word) const;

  private:
    const tatar_info& info;
    const fst g2p_fst;
    const fst g2p_p_fst;
    const fst lseq_fst;
  };
}
#endif
