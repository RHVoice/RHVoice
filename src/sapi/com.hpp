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

#ifndef RHVOICE_SAPI_COM_HPP
#define RHVOICE_SAPI_COM_HPP

#include <vector>
#include <string>
#include <memory>
#include <new>
#include <stdexcept>
#include <windows.h>
#include <initguid.h>
#include <comdef.h>

#include "registry.hpp"
#include "utils.hpp"

namespace RHVoice
{
  namespace com
  {
    wchar_t* strdup(const std::wstring& s);

    std::wstring clsid_as_string(REFCLSID clsid);

    template<class T>
    std::wstring clsid_as_string()
    {
      return clsid_as_string(__uuidof(T));
    }

    class object_counter
    {
    public:
      static void increment()
      {
        InterlockedIncrement(&count);
      }

      static void decrement()
      {
        InterlockedDecrement(&count);
      }

      static bool is_zero()
      {
        return (InterlockedCompareExchange(&count,0,0)==0);
      }

    private:
      static volatile long count;
    };

    template<class I>
    inline void* try_interface(void* ptr,REFIID riidI,REFIID riid)
    {
      return (IsEqualIID(riid,riidI)?static_cast<I*>(ptr):0);
    }

    template<class I,class O>
    inline void* try_interface(O* ptr,REFIID riid)
    {
      return try_interface<I>(ptr,__uuidof(I), riid);
    }

    template<class I>
    inline void* try_primary_interface(void* ptr, REFIID riidI,REFIID riid)
    {
      if(IsEqualIID(riid,IID_IUnknown))
        return static_cast<IUnknown*>(static_cast<I*>(ptr));
      else if(IsEqualIID(riid,riidI))
        return static_cast<I*>(ptr);
      else
        return 0;
    }

    template<class I,class O>
    inline void* try_primary_interface(O* ptr,REFIID riid)
    {
      return try_primary_interface<I>(ptr, __uuidof(I), riid);
    }

    template<class T>
    class object;

    template <class T>
    class IUnknownImpl: public T
    {
      friend class object<T>;
    private:
      IUnknownImpl():
        ref_count(1)
      {
      }

      template<typename A>
      explicit IUnknownImpl(const A& arg):
        T(arg),
        ref_count(1)
      {
      }

      ~IUnknownImpl()
      {
        object_counter::decrement();
      }

      volatile long ref_count;

    public:
      STDMETHOD_(ULONG,AddRef)();
      STDMETHOD_(ULONG,Release)();
      STDMETHOD(QueryInterface)(REFIID riid,void** ppv);
    };

    template<class T>
    STDMETHODIMP_(ULONG) IUnknownImpl<T>::AddRef()
    {
      return InterlockedIncrement(&ref_count);
    }

    template<class T>
    STDMETHODIMP_(ULONG) IUnknownImpl<T>::Release()
    {
      long count=InterlockedDecrement(&ref_count);
      if(count==0)
        delete this;
      return count;
    }

    template<class T>
    STDMETHODIMP IUnknownImpl<T>::QueryInterface(REFIID riid,void** ppv)
    {
      if(ppv==0)
        return E_POINTER;
      void* pv=this->get_interface(riid);
      *ppv=pv;
      if(pv==0)
        return E_NOINTERFACE;
      else
        {
          AddRef();
          return S_OK;
        }
    }

    template<class T>
    class object
    {
    public:
      object():
        impl(new IUnknownImpl<T>)
      {
        object_counter::increment();
      }

      template<typename A>
      object(const A& arg):
        impl(new IUnknownImpl<T>(arg))
      {
        object_counter::increment();
      }

      ~object()
      {
        impl->Release();
      }

      IUnknownImpl<T>* operator->() const
      {
        return impl;
      }

    private:
      object(const object&);
      object& operator=(const object&);

      IUnknownImpl<T>* impl;
    };

    template<class I>
    class interface_ptr
    {
    public:
      interface_ptr():
        ptr(0)
      {
      }

      template<class T>
      explicit interface_ptr(object<T>& obj, REFIID riid)
      {
        if(FAILED(obj->QueryInterface(riid,reinterpret_cast<void**>(&ptr))))
          throw std::invalid_argument("Unsupported interface");
      }

      interface_ptr(const interface_ptr& other):
        ptr(other.ptr)
      {
        if(ptr)
        ptr->AddRef();
      }

      interface_ptr& operator=(const interface_ptr& other)
      {
        if(other.ptr)
          other.ptr->AddRef();
        release();
        ptr=other.ptr;
      }

      ~interface_ptr()
      {
        release();
      }

      I* get(bool add_ref=true)
      {
        if(ptr&&add_ref)
          ptr->AddRef();
        return ptr;
      }

    private:
      void release()
      {
        if(ptr)
          {
            ptr->Release();
            ptr=0;
          }
      }

      I* ptr;
    };

    template<class T>
    class IClassFactoryImpl: public IClassFactory
    {
    protected:
      void* get_interface(REFIID riid)
      {
        return try_primary_interface<IClassFactory>(this, IID_IClassFactory, riid);
      }

    public:
      STDMETHOD(CreateInstance)(IUnknown* pUnkOuter,REFIID riid,void** ppvect);
      STDMETHOD(LockServer)(BOOL fLock);
    };

    template<class T>
    STDMETHODIMP IClassFactoryImpl<T>::CreateInstance(IUnknown* pUnkOuter,REFIID riid,void** ppv)
    {
      if(pUnkOuter!=0)
        return CLASS_E_NOAGGREGATION;
      if(ppv==0)
        return E_POINTER;
      *ppv=0;
      try
        {
          object<T> obj;
          return obj->QueryInterface(riid,ppv);
        }
      catch(const std::bad_alloc& e)
        {
          return E_OUTOFMEMORY;
        }
      catch(...)
        {
          return E_UNEXPECTED;
        }
    }

    template<class T>
    STDMETHODIMP IClassFactoryImpl<T>::LockServer(BOOL fLock)
    {
      if(fLock)
        object_counter::increment();
      else
        object_counter::decrement();
      return S_OK;
    }

    class proto_class_object_factory
    {
    protected:
      class creator;
      typedef std::shared_ptr<creator> creator_ptr;
    public:
      proto_class_object_factory();

      HRESULT create(REFCLSID rclsid,REFIID riid,void** ppv) const;

      void register_class(creator *c);

      class creator
      {
      public:
        virtual ~creator();

        virtual bool matches(REFCLSID rclsid) const=0;
        virtual HRESULT create(REFIID riid,void** ppv) const=0;
      };

      class proto_concrete_creator: public creator
      {
      public:
        proto_concrete_creator(REFIID rclsid);

        virtual bool matches(REFCLSID rclsid) const;
      private:
        GUID own;
      };

      proto_class_object_factory(const proto_class_object_factory&);
      proto_class_object_factory& operator=(const proto_class_object_factory&);

      std::vector<creator_ptr> creators;
    };

    class class_object_factory: public proto_class_object_factory
    {
    public:
      class_object_factory(): proto_class_object_factory()
      {
      }

      template<class T> void register_class()
      {
        register_class(__uuidof(T));
      }

      template<class T> void register_class(REFIID uuid)
      {
        proto_class_object_factory::register_class(static_cast<creator*>(new concrete_creator<T>(uuid)));
      }

    private:
      class_object_factory(const class_object_factory&) = default;
      class_object_factory& operator=(const class_object_factory&) = default;

      template<class T>
      class concrete_creator: public proto_concrete_creator
      {
      public:
        concrete_creator(REFIID uuid);
        concrete_creator();
        virtual HRESULT create(REFIID riid,void** ppv) const;
      };
    };

    template<class T>
    class_object_factory::concrete_creator<T>::concrete_creator(REFIID uuid): proto_concrete_creator(uuid){};
    template<class T>
    class_object_factory::concrete_creator<T>::concrete_creator(): concrete_creator(__uuidof(T)){};

    template<class T>
    HRESULT class_object_factory::concrete_creator<T>::create(REFIID riid,void** ppv) const
    {
      object<IClassFactoryImpl<T> > obj;
      return obj->QueryInterface(riid,ppv);
    }

    class class_registrar
    {
    public:
      explicit class_registrar(HINSTANCE dll_handle);
      void register_class(std::wstring str_clsid);
      void unregister_class(std::wstring str_clsid);
      void register_class(REFIID str_clsid);
      void unregister_class(REFIID str_clsid);
      template<class T> void register_class();
      template<class T> void unregister_class();

    private:
      class_registrar(const class_registrar&);
      class_registrar& operator=(const class_registrar&);

      std::wstring dll_path;
      static const std::wstring clsid_key_path;
    };

    template<class T> void class_registrar::register_class()
    {
      register_class(__uuidof(T));
    }

    template<class T> void class_registrar::unregister_class()
    {
      unregister_class(__uuidof(T));
    }
  }
}
#endif
