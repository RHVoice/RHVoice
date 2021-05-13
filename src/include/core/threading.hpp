/* Copyright (C) 2012  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_THREADING_HPP
#define RHVOICE_THREADING_HPP

#ifdef WIN32
#include <windows.h>
#else
#include <deque>
#include <pthread.h>
#include "exception.hpp"
#endif

namespace RHVoice
{
  namespace threading
  {
  #ifndef WIN32
    class mutex_init_error: public exception
    {
    public:
      mutex_init_error():
        exception("Mutex initialization failed")
      {
      }
    };
  #endif

    class mutex
    {
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

#ifdef WIN32
      typedef CRITICAL_SECTION native_mutex_type;
#else
      typedef pthread_mutex_t native_mutex_type;
#endif

      native_mutex_type* get_native_mutex()
      {
        return &native_mutex;
      }

    private:
      mutex(const mutex&);
      mutex& operator=(const mutex&);

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

    #ifndef WIN32
    class cond_init_error: public exception
    {
    public:
      cond_init_error():
        exception("Condition variable initialization failed")
      {
      }
    };

    class cond_wait_error: public exception
    {
    public:
      cond_wait_error():
        exception("Condition variable wait failed")
      {
      }
    };

    class condition_variable
    {
    public:
      condition_variable()
      {
        if(pthread_cond_init(&native_cond,0)!=0)
          throw cond_init_error();
      }

      ~condition_variable()
      {
        pthread_cond_destroy(&native_cond);
      }

      void signal()
      {
        pthread_cond_signal(&native_cond);
      }

      void broadcast()
      {
        pthread_cond_broadcast(&native_cond);
      }

      void wait(mutex& m)
      {
        if(pthread_cond_wait(&native_cond,m.get_native_mutex())!=0)
          throw cond_wait_error();
      }

    private:
      pthread_cond_t native_cond;
    };

  template<typename T>
  class queue
  {
  public:
    queue():
      stopped(false)
    {
    }

    void push(const T& elem);
    bool pop(T& val);
    void stop();
    void clear();

  private:
    bool stopped;
    std::deque<T> data;
    mutex data_mutex;
    condition_variable no_longer_empty;
  };

  template<typename T>
  void queue<T>::push(const T& elem)
  {
    lock data_lock(data_mutex);
    if(stopped)
      return;
    bool was_empty=data.empty();
    data.push_back(elem);
    if(was_empty)
      no_longer_empty.signal();
  }

  template<typename T>
  bool queue<T>::pop(T& val)
  {
    lock data_lock(data_mutex);
    if(stopped)
      return false;
    while(data.empty())
      {
        no_longer_empty.wait(data_mutex);
        if(stopped)
          return false;
      }
    val=data.front();
    data.pop_front();
    return true;
  }

    template<typename T>
    void queue<T>::stop()
    {
      lock data_lock(data_mutex);
      stopped=true;
      no_longer_empty.signal();
    }

    template<typename T>
    void queue<T>::clear()
    {
      lock data_lock(data_mutex);
      data.clear();
    }

    class thread_creation_error: public exception
    {
    public:
      thread_creation_error():
        exception("Thread creation failed")
      {
      }
    };

    class thread
    {
    public:
      virtual ~thread()
      {
      }

      void start()
      {
        prepare();
        if(pthread_create(&id,0,&do_start,this)!=0)
          throw thread_creation_error();
        started=true;
      }

      void join()
      {
        if(!started)
          return;
        pthread_join(id,0);
        started=false;
      }

      bool has_started() const
      {
        return started;
      }

    protected:
      thread():
        started(false)
      {
      }

    private:
      thread(const thread&);
      thread& operator=(const thread&);

      virtual void prepare()
      {
      }

      virtual void run()=0;

      static void* do_start(void* arg)
      {
        static_cast<thread*>(arg)->run();
        return 0;
      }

      pthread_t id;
      bool started;
    };
#endif
  }
}
#endif
