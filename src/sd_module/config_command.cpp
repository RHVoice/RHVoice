/* Copyright (C) 2012, 2018  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 2 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#include "core/str.hpp"
#include "io.hpp"
#include "config_command.hpp"

namespace RHVoice
{
  namespace sd
  {
    void config_command::read_settings()
    {
      for(std::string line(getline());line!=".";line=getline())
        {
          str::tokenizer<str::is_equal_to> tok(line,str::is_equal_to('='));
          str::tokenizer<str::is_equal_to>::iterator it=tok.begin();
          if(it!=tok.end())
            {
              std::string name(*it);
              ++it;
              if(it!=tok.end())
                {
                  logger::log(4,"Setting ",name," to ",*it);
                  if(*it=="NULL")
                    conf.reset(name);
                  else
                    {
                      bool result=conf.set(name,*it);
                      if(!result)
                        logger::log(4,"Unsupported setting or value");
                    }
                }
            }
        }
    }
  }
}
