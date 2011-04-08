/* Copyright (C) 2011  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <unistr.h>
#include "ustring.h"
#include "vector.h"

vector_t(uint32_t,uchars32)

vector_t(uint8_t,uchars8)

static uint32_t cz32='\0';
static uint8_t cz8='\0';

ustring32_t ustring32_alloc(size_t min_size)
{
  ustring32_t u=uchars32_alloc(min_size+1,NULL);
  if(u==NULL) return NULL;
  uchars32_push(u,&cz32);
  return u;
}

ustring8_t ustring8_alloc(size_t min_size)
{
  ustring8_t u=uchars8_alloc(min_size+1,NULL);
  if(u==NULL) return NULL;
  uchars8_push(u,&cz8);
  return u;
}

void ustring32_free(ustring32_t u)
{
  uchars32_free(u);
}

void ustring8_free(ustring8_t u)
{
  uchars8_free(u);
}

size_t ustring32_length(ustring32_t u)
{
  return (uchars32_size(u)-1);
}

uint32_t *ustring32_str(ustring32_t u)
{
  return uchars32_data(u);
}

int ustring32_empty(ustring32_t u)
{
  return (uchars32_size(u)==1);
}

size_t ustring8_length(ustring8_t u)
{
  return (uchars8_size(u)-1);
}

uint8_t *ustring8_str(ustring8_t u)
{
  return uchars8_data(u);
}

int ustring8_empty(ustring8_t u)
{
  return (uchars8_size(u)==1);
}

ustring32_t ustring32_push(ustring32_t u,ucs4_t c)
{
  if(!uchars32_push(u,&cz32)) return NULL;
  uchars32_data(u)[uchars32_size(u)-2]=c;
  return u;
}

ustring8_t ustring8_push(ustring8_t u,ucs4_t c)
{
  uint8_t b[8]={'\0'};
  int n=u8_uctomb(b,c,8);
  if(n<=0) return NULL;
  if(!uchars8_reserve(u,uchars8_size(u)+n)) return NULL;
  uchars8_pop(u);
  uchars8_append(u,b,n+1);
  return u;
}

void ustring32_clear(ustring32_t u)
{
  uchars32_clear(u);
  uchars32_push(u,&cz32);
}

ustring32_t ustring32_reserve(ustring32_t u,size_t r)
{
  return uchars32_reserve(u,r+1);
}

void ustring8_clear(ustring8_t u)
{
  uchars8_clear(u);
  uchars8_push(u,&cz8);
}

ustring8_t ustring8_reserve(ustring8_t u,size_t r)
{
  return uchars8_reserve(u,r+1);
}

ustring8_t ustring32_substr8(ustring8_t u8,ustring32_t u32,size_t p,size_t l)
{
  if((u8==NULL)||(u32==NULL)) return NULL;
  size_t s=ustring32_length(u32);
  if((s==0)&&(p==0))
    {
      ustring8_clear(u8);
      return u8;
    }
  if(p>=s) return NULL;
  size_t n=s-p;
  if((l>0)&&(l<n)) n=l;
if(!uchars8_reserve(u8,uchars8_size(u8)+n)) return NULL;
  ustring8_clear(u8);
  const uint32_t *str=ustring32_str(u32);
  str+=p;
  size_t i;
  for(i=0;i<n;i++)
    {
      if(!ustring8_push(u8,str[i])) return NULL;
    }
  return u8;
}

ustring32_t ustring32_assign8(ustring32_t u32,const uint8_t *u8)
{
  ustring32_clear(u32);
  ucs4_t c;
  const uint8_t *s=u8;
  while((s=u8_next(&c,s)))
    {
      if(!ustring32_push(u32,c)) return NULL;
    }
  return u32;
}

ucs4_t ustring32_at(ustring32_t u,size_t i)
{
  ucs4_t *pc=uchars32_at(u,i);
  return (pc==NULL)?0xffffffff:(*pc);
}

ustring32_t ustring32_set(ustring32_t u,size_t p,ucs4_t c)
{
  if(p>=ustring32_length(u)) return NULL;
  ustring32_str(u)[p]=c;
  return u;
}

ustring8_t ustring8_append(ustring8_t u,const uint8_t *s,size_t n)
{
  if((u==NULL)||(s==NULL)||(n==0)) return u;
  if(!uchars8_reserve(u,uchars8_size(u)+n)) return NULL;
  uchars8_pop(u);
  uchars8_append(u,(uint8_t*)s,n);
  uchars8_push(u,&cz8);
  return u;
}

ustring8_t ustring8_assign(ustring8_t u,const uint8_t *s,size_t n)
{
  if((u==NULL)||(s==NULL)||(n==0)) return u;
  if(!uchars8_reserve(u,n+1)) return NULL;
  uchars8_clear(u);
  uchars8_append(u,(uint8_t*)s,n);
  uchars8_push(u,&cz8);
  return u;
}
