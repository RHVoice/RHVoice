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

#ifndef RHVOICE_VALUE_HPP
#define RHVOICE_VALUE_HPP

#include <string>
#include <iostream>
#include <map>

namespace RHVoice
{
  class value
  {
    friend std::ostream& operator<<(std::ostream&,const value&);
  private:
    struct container
    {
      virtual ~container()
      {
      }

      virtual container* clone() const=0;
      virtual std::ostream& print(std::ostream&) const=0;
    };

    template<typename T>
    struct concrete_container: public container
    {
      explicit concrete_container(const T& obj):
      object(obj)
      {
      }

      concrete_container* clone() const
      {
        return new concrete_container(object);
      }

      std::ostream& print(std::ostream& out) const
      {
        return (out << object);
      }

      T object;
    };

  public:
    value():
      object_container(0)
    {
    }

    template<typename T>
    value(const T& obj):
    object_container(new concrete_container<T>(obj))
    {
    }

    ~value()
    {
      delete object_container;
    }

    value(const value& other):
    object_container((other.object_container)?(other.object_container->clone()):0)
    {
    }

    value& operator=(const value& other)
    {
      container* tmp=(other.object_container)?(other.object_container->clone()):0;
      delete object_container;
      object_container=tmp;
      return *this;
    }

    bool empty() const
    {
      return (object_container==0);
    }

    template<typename T>
    const T& as() const
    {
      return (dynamic_cast<concrete_container<T>&>(*object_container).object);
    }

    template<typename T>
    bool is() const
    {
      return (dynamic_cast<concrete_container<T>*>(object_container));
    }

  private:
    container* object_container;
  };

  inline std::ostream& operator<<(std::ostream& out,const value& val)
  {
    return (val.object_container)?(val.object_container->print(out)):out;
  }
}
#endif
