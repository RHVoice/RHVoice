/* Copyright (C) 2013, 2018  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_GEORGIAN_HPP
#define RHVOICE_GEORGIAN_HPP

#include "str.hpp"
#include "fst.hpp"
#include "language.hpp"

namespace RHVoice
{
  class georgian_info: public language_info
  {
  public:
    georgian_info(const std::string& data_path,const std::string& userdict_path);

    bool has_unicase_alphabet() const
    {
      return true;
    }

    #ifdef WIN32
    unsigned short get_id() const
    {
      return 1079;
    }
    #endif

    std::string get_country() const
    {
      return "GEO/GE";
    }

    bool supports_pseudo_english() const
    {
      return true;
    }

  private:
    std::shared_ptr<language> create_instance() const;
  };

  class georgian: public language
  {
  public:
    explicit georgian(const georgian_info& info_);

    const georgian_info& get_info() const
    {
      return info;
    }

    std::vector<std::string> get_word_transcription(const item& word) const;

  private:
    const georgian_info& info;
    const fst g2p_fst;
  };
}
#endif
