/* Copyright (C) 2021  Olga Yakovleva <olga@rhvoice.org> */

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

#ifndef RHVOICE_PKG_URL_BUILDER_HPP
#define RHVOICE_PKG_URL_BUILDER_HPP

#include <string>
#include <map>
#include "boost/utility/string_view.hpp"

namespace RHVoice
{
  namespace pkg
  {
    class url_builder final
    {
    public:
      url_builder()=default;
      std::string build() const;

      void set_base_url(const std::string& b)
      {
        base_url=b;
      }

      void set_base_url(boost::string_view b)
      {
        base_url.assign(b.data(), b.size());
      }

      void set_path(const std::string& p)
      {
        rel_path=p;
      }

      void set_path(boost::string_view p)
      {
        rel_path.assign(p.data(), p.size());
      }


    private:
      std::string base_url;
      std::string rel_path;
    };
  }
}
#endif
