/* Copyright (C) 2012, 2013  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_INI_PARSER_HPP
#define RHVOICE_INI_PARSER_HPP

#include <memory>
#include <fstream>
#include <string>

namespace RHVoice
{
  class ini_parser
  {
  public:
    explicit ini_parser(const std::string& file_path,bool standard=true);
    void next();

    bool done() const
    {
      return (instream.get()==0);
    }

    const std::string& get_section() const
    {
      return section;
    }

    const std::string& get_key() const
    {
      return key;
    }

    const std::string& get_value() const
    {
      return value;
    }

  private:
    ini_parser(const ini_parser&);
    ini_parser& operator=(const ini_parser&);
    std::string unescape(const std::string& s) const;

    std::unique_ptr<std::ifstream> instream;
    bool standard_format;
    std::string section,key,value;
  };
}
#endif
