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

#include "core/relation.hpp"
#include "core/item.hpp"
#include "core/utterance.hpp"

namespace RHVoice
{
  item& relation::append(item& existing_item)
  {
    if(tail)
      return tail->append(existing_item);
    else
      {
        head=tail=new item(existing_item,this);
        return *tail;
      }
  }

  item& relation::append()
  {
    if(tail)
      return tail->append();
    else
      {
        head=tail=new item(this);
        return *tail;
      }
  }

  item& relation::prepend(item& existing_item)
  {
    if(head)
      return head->prepend(existing_item);
    else
      {
        head=tail=new item(existing_item,this);
        return *head;
      }
  }

  item& relation::prepend()
  {
    if(head)
      return head->prepend();
    else
      {
        head=tail=new item(this);
        return *head;
      }
  }

  void relation::clear()
  {
    while(!empty())
      {
        head->remove();
      }
  }
}
