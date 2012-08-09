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

#ifndef RHVOICE_MUTEX_HPP
#define RHVOICE_MUTEX_HPP

#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#include "exception.hpp"
#endif

namespace RHVoice
{
  #ifndef WIN32
  class mutex_init_error: public exception
  {
  public:
    explicit mutex_init_error():
      exception("Mutex initialization failed")
    {
    }
  };
  #endif

  class lock;

  class mutex
  {
    friend class lock;
  public:
    mutex()
    {
      #ifdef WIN32
      InitializeCriticalSection(&native_mutex);
      #else
      if(pthread_mutex_init(&native_mutex,0)!=0)
        throw mutex_init_error();
      #endif
    }

    ~mutex()
    {
      #ifdef WIN32
      DeleteCriticalSection(&native_mutex);
      #else
      pthread_mutex_destroy(&native_mutex);
      #endif
    }

  private:
    void lock()
    {
      #ifdef WIN32
      EnterCriticalSection(&native_mutex);
      #else
      pthread_mutex_lock(&native_mutex);
      #endif
    }

    void unlock()
    {
      #ifdef WIN32
      LeaveCriticalSection(&native_mutex);
      #else
      pthread_mutex_unlock(&native_mutex);
      #endif
    }

    mutex(const mutex&);
    mutex& operator=(const mutex&);

    #ifdef WIN32
    typedef CRITICAL_SECTION native_mutex_type;
    #else
    typedef pthread_mutex_t native_mutex_type;
    #endif
    native_mutex_type native_mutex;
  };

  class lock
  {
  public:
    explicit lock(mutex& m):
      locked_mutex(m)
    {
      locked_mutex.lock();
    }

    ~lock()
    {
      locked_mutex.unlock();
    }
  private:
    mutex& locked_mutex;
  };
}
#endif
