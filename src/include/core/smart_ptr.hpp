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
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef RHVOICE_SMART_PTR_HPP
#define RHVOICE_SMART_PTR_HPP

#ifdef WIN32
#include <windows.h>
#endif

namespace RHVoice
{
  template<typename T>
  class smart_ptr
  {
  private:
    struct default_deleter
    {
      void operator()(T* p) const
      {
        delete p;
      }
    };

    class ref_manager_base
    {
    public:
      virtual ~ref_manager_base()
      {
      }

      int increment()
      {
      #ifdef WIN32
        return InterlockedIncrement(&count);
      #else
        return __sync_add_and_fetch(&count,1);
      #endif
      }

      int decrement()
      {
      #ifdef WIN32
        return InterlockedDecrement(&count);
      #else
        return __sync_sub_and_fetch(&count,1);
      #endif
      }

      virtual void destroy(T* p) const =0;
    protected:
      ref_manager_base():
        count(1)
      {
      }
    private:
      ref_manager_base(const ref_manager_base&);
      ref_manager_base& operator=(const ref_manager_base&);

      #ifdef WIN32
      volatile long count;
      #else
      volatile int count;
      #endif
    };

    template<typename D>
    class ref_manager: public ref_manager_base
    {
    public:
      explicit ref_manager(D d):
      del(d)
      {
      }
      void destroy(T* p) const
      {
        if(p!=0)
          del(p);
      }
    private:
      D del;
    };

  public:
    smart_ptr():
      ptr(0),
      rm(0)
    {
    }
    explicit smart_ptr(T* p);
    template <typename D>
    smart_ptr(T* p,D d);

    smart_ptr(const smart_ptr& other)
    {
      const_cast<smart_ptr<T>&>(other).add_ref();
      ptr=other.ptr;
      rm=other.rm;
    }

    smart_ptr& operator=(const smart_ptr& other);

    void reset()
    {
      release();
      ptr=0;
      rm=0;
    }

    void reset(T* p);
    template<typename D>
    void reset(T* p,D d);

    ~smart_ptr()
    {
      release();
    }

    bool empty() const
    {
      return (ptr==0);
    }

    T& operator*()
  {
    return *ptr;
  }

    const T& operator*() const
  {
    return *ptr;
  }

    T* operator->()
  {
    return ptr;
  }

    const T* operator->() const
    {
      return ptr;
    }

    T* get()
  {
    return ptr;
  }

    const T* get() const
    {
      return ptr;
    }
  private:
    void add_ref()
    {
      if(rm!=0)
        rm->increment();
    }
    void release();
    T* ptr;
    ref_manager_base* rm;
  };

  template<typename T>
  void smart_ptr<T>::release()
  {
    if(rm!=0)
      {
        if(rm->decrement()==0)
          {
            rm->destroy(ptr);
            delete rm;
          }
      }
  }

  template<typename T>
  smart_ptr<T>::smart_ptr(T* p):
  ptr(p),
  rm(0)
  {
    try
      {
        rm=new ref_manager<default_deleter>(default_deleter());
      }
    catch(...)
      {
        delete p;
        throw;
      }
  }

  template<typename T> template<typename D>
  smart_ptr<T>::smart_ptr(T* p,D d):
  ptr(p),
  rm(0)
  {
    try
      {
        rm=new ref_manager<D>(d);
      }
    catch(...)
      {
        if(p)
          d(p);
        throw;
      }
  }

  template<typename T>
  smart_ptr<T>& smart_ptr<T>::operator=(const smart_ptr<T>& other)
  {
    if(this==&other)
      return *this;
    release();
    const_cast<smart_ptr<T>&>(other).add_ref();
    ptr=other.ptr;
    rm=other.rm;
    return *this;
  }

  template<typename T>
  void smart_ptr<T>::reset(T* p)
  {
    ref_manager_base* new_rm=0;
    try
      {
        new_rm=new ref_manager<default_deleter>(default_deleter());
      }
    catch(...)
      {
        delete p;
        throw;
      }
    release();
    ptr=p;
    rm=new_rm;
  }

  template<typename T> template<typename D>
  void smart_ptr<T>::reset(T* p,D d)
  {
    ref_manager_base* new_rm=0;
    try
      {
        new_rm=new ref_manager<D>(d);
      }
    catch(...)
        {
          if(p!=0)
            d(p);
          throw;
        }
    release();
    ptr=p;
    rm=new_rm;
  }
}
#endif
