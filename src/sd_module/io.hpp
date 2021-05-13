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

#ifndef RHVOICE_SD_IO_HPP
#define RHVOICE_SD_IO_HPP

#include <ostream>
#include "core/threading.hpp"
#include "core/property.hpp"
#include "error.hpp"

namespace RHVoice
{
  namespace sd
  {
    struct broken_pipe: public error
    {
      broken_pipe():
        error("Broken pipe")
      {
      }
    };

    template<class T>
    class basic_message
    {
    protected:
      basic_message(std::ostream& s,bool throw_exceptions_):
        stream(s),
        throw_exceptions(throw_exceptions_),
        stream_lock(stream_mutex)
      {
      }

      ~basic_message()
      {
      }

    public:
      template<typename A>
      void operator()(const A& arg)
      {
        start();
        stream << arg << std::endl;
        flash();
      }

      template<typename A1,typename A2>
      void operator()(const A1& arg1,const A2& arg2)
      {
        start();
        stream << arg1 << arg2 << std::endl;
        flash();
      }

      template<typename A1,typename A2,typename A3>
      void operator()(const A1& arg1,const A2& arg2,const A3& arg3)
      {
        start();
        stream << arg1 << arg2 << arg3 << std::endl;
        flash();
      }

      template<typename A1,typename A2,typename A3,typename A4>
      void operator()(const A1& arg1,const A2& arg2,const A3& arg3,const A4& arg4)
      {
        start();
        stream << arg1 << arg2 << arg3 << arg4 << std::endl;
        flash();
      }

      template<typename A1,typename A2,typename A3,typename A4,typename A5>
      void operator()(const A1& arg1,const A2& arg2,const A3& arg3,const A4& arg4,const A5& arg5)
      {
        start();
        stream << arg1 << arg2 << arg3 << arg4 << arg5 << std::endl;
        flash();
      }

      template<typename A1,typename A2,typename A3,typename A4,typename A5,typename A6>
      void operator()(const A1& arg1,const A2& arg2,const A3& arg3,const A4& arg4,const A5& arg5,const A6& arg6)
      {
        start();
        stream << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 << std::endl;
        flash();
      }

    protected:
      std::ostream& stream;

    private:
      basic_message(const basic_message&);
      basic_message& operator=(const basic_message&);

      void flash()
      {
        stream.flush();
        if(!stream)
          if(throw_exceptions)
            throw broken_pipe();
      }

      virtual void start()
      {
}

      static threading::mutex stream_mutex;

      bool throw_exceptions;
      threading::lock stream_lock;
    };

    template<class T>
    threading::mutex basic_message<T>::stream_mutex;

    class message: public basic_message<message>
    {
    public:
      message();

    protected:
      message(bool throw_exceptions);
    };

    class reply: public message
    {
    public:
      reply():
        message(true)
      {
      }
    };

    class logger
    {
    public:
      static numeric_property<unsigned int> log_level;

      template<typename A>
      static void log(unsigned int level,const A& arg)
      {
        if(level<=log_level)
          log_message()(arg);
      }

      template<typename A1,typename A2>
      static void log(unsigned int level,const A1& arg1,const A2& arg2)
      {
        if(level<=log_level)
          log_message()(arg1,arg2);
      }

      template<typename A1,typename A2,typename A3>
      static void log(unsigned int level,const A1& arg1,const A2& arg2,const A3& arg3)
      {
        if(level<=log_level)
          log_message()(arg1,arg2,arg3);
      }

      template<typename A1,typename A2,typename A3,typename A4>
      static void log(unsigned int level,const A1& arg1,const A2& arg2,const A3& arg3,const A4& arg4)
      {
        if(level<=log_level)
          log_message()(arg1,arg2,arg3,arg4);
      }

      template<typename A1,typename A2,typename A3,typename A4,typename A5>
      static void log(unsigned int level,const A1& arg1,const A2& arg2,const A3& arg3,const A4& arg4,const A5& arg5)
      {
        if(level<=log_level)
          log_message()(arg1,arg2,arg3,arg4,arg5);
      }

    private:
      logger();

      class log_message: public basic_message<log_message>
      {
      public:
        log_message();

      private:
        void start();
      };
    };

    std::string getline();
  }
}
#endif
