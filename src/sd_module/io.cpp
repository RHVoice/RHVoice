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

#define _POSIX_C_SOURCE 1
#include <iostream>
#include <time.h>
#include <string.h>
#include "io.hpp"

namespace RHVoice
{
  namespace sd
  {
    message::message():
      basic_message<message>(std::cout,false)
    {
    }

    message::message(bool throw_exceptions):
      basic_message<message>(std::cout,throw_exceptions)
    {
    }

    std::string getline()
    {
      std::string line;
      if(std::getline(std::cin,line))
        return line;
      else
        throw broken_pipe();
    }

    numeric_property<unsigned int> logger::log_level("log_level",3,0,5);

    logger::log_message::log_message():
      basic_message<log_message>(std::clog,false)
    {
    }

    void logger::log_message::start()
    {
      time_t t=time(0);
      char s[32];
      if(ctime_r(&t,s)==0)
        return;
      char* p=strchr(s,'\n');
      if(p!=0)
        *p=' ';
      stream <<s << ' ';
}
  }
}
