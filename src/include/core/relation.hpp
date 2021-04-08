/* Copyright (C) 2012, 2019  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_RELATION_HPP
#define RHVOICE_RELATION_HPP

#include <string>
#include <iterator>
#include "exception.hpp"
#include "item.hpp"


namespace RHVoice
{
  class utterance;

  class relation
  {
    friend class item;
    friend class item::iterator;
    friend class item::const_iterator;
    friend class utterance;
  private:
    relation(const std::string& name_,utterance& utterance_ref_):
      name(name_),
      utterance_ref(utterance_ref_),
      head(0),
      tail(0)
    {
    }

  public:
    ~relation()
    {
      clear();
    }

    const std::string& get_name() const
    {
      return name;
    }

    const utterance& get_utterance() const
    {
      return utterance_ref;
    }

    utterance& get_utterance()
    {
      return utterance_ref;
    }

    const item& first() const
    {
      if(head==0)
        throw item_not_found();
      return *head;
    }

    const item* first_ptr() const
    {
      return head;
}

    item& first()
    {
      return const_cast<item&>(static_cast<const relation&>(*this).first());
    }

    item* first_ptr()
    {
      return head;
}

    const item& last() const
    {
      if(tail==0)
        throw item_not_found();
      return *tail;
    }

    const item* last_ptr() const
    {
      return tail;
}

    item& last()
    {
      return const_cast<item&>(static_cast<const relation&>(*this).last());
    }

    item* last_ptr()
    {
      return tail;
}

    bool empty() const
    {
      return (head==0);
    }

  private:
    const std::string name;
    utterance& utterance_ref;
    item *head,*tail;

  public:
    item& append(item& existing_item);
    item& append();
    item& prepend(item& existing_item);
    item& prepend();
    void clear();

    typedef item::iterator iterator;

    iterator begin()
    {
      return iterator(head,0,this);
    }

    iterator end()
    {
      return iterator(0,0,this);
    }

    typedef item::const_iterator const_iterator;

    const_iterator begin() const
    {
      return const_iterator(head,0,this);
    }

    const_iterator end() const
    {
      return const_iterator(0,0,this);
    }

    typedef std::reverse_iterator<iterator> reverse_iterator;

    reverse_iterator rbegin()
    {
      return reverse_iterator(end());
    }

    reverse_iterator rend()
    {
      return reverse_iterator(begin());
    }

    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    const_reverse_iterator rbegin() const
    {
      return const_reverse_iterator(end());
    }

    const_reverse_iterator rend() const
    {
      return const_reverse_iterator(begin());
    }

    class back_insert_iterator: public std::iterator<std::output_iterator_tag,void,void,void,void>
    {
    public:
      back_insert_iterator():
        target_relation(0)
      {
      }

      back_insert_iterator(const back_insert_iterator& other):
      target_relation(other.target_relation)
      {
      }

      explicit back_insert_iterator(relation* rel):
      target_relation(rel)
      {
      }

      back_insert_iterator& operator=(const back_insert_iterator& other)
      {
        target_relation=other.target_relation;
        return *this;
      }

      back_insert_iterator& operator=(item& value)
      {
        target_relation->append(value);
        return *this;
      }

      back_insert_iterator& operator=(const std::string& value)
      {
        target_relation->append().set("name",value);
        return *this;
      }

      back_insert_iterator& operator*()
      {
        return *this;
      }

      back_insert_iterator& operator++()
      {
        return *this;
      }

      back_insert_iterator& operator++(int)
      {
        return *this;
      }

    private:
      relation* target_relation;
    };

    back_insert_iterator back_inserter()
    {
      return back_insert_iterator(this);
    }

    class front_insert_iterator: public std::iterator<std::output_iterator_tag,void,void,void,void>
    {
    public:
      front_insert_iterator():
        target_relation(0)
      {
      }

      front_insert_iterator(const front_insert_iterator& other):
      target_relation(other.target_relation)
      {
      }

      explicit front_insert_iterator(relation* rel):
      target_relation(rel)
      {
      }

      front_insert_iterator& operator=(const front_insert_iterator& other)
      {
        target_relation=other.target_relation;
        return *this;
      }

      front_insert_iterator& operator=(item& value)
      {
        target_relation->prepend(value);
        return *this;
      }

      front_insert_iterator& operator=(const std::string& value)
      {
        target_relation->prepend().set("name",value);
        return *this;
      }

      front_insert_iterator& operator*()
      {
        return *this;
      }

      front_insert_iterator& operator++()
      {
        return *this;
      }

      front_insert_iterator& operator++(int)
      {
        return *this;
      }

    private:
      relation* target_relation;
    };

    front_insert_iterator front_inserter()
    {
      return front_insert_iterator(this);
    }
  };
}
#endif
