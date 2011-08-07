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

#include <stdlib.h>
#include <search.h>
#include <unistr.h>
#include <unicase.h>
#include "tst.h"
#include "vector.h"

typedef struct
{
  ucs4_t spltchr;
  uint32_t left,right,mid;
} node;

vector_t(node,nodelist)

static node default_node={'\0',0,0,0};

typedef struct
{
  uint8_t *key;
  void *value;
  uint32_t index;
} item;

vector_t(item,itemlist)

struct tst_s
{
  nodelist nodes;
  itemlist items;
  tst_free_value_func free_value_func;
};

tst tst_create(tst_free_value_func free_value_func)
{
  tst t=(tst)malloc(sizeof(struct tst_s));
  if(t==NULL) goto err0;
  t->nodes=nodelist_alloc(1,NULL);
  if(t->nodes==NULL) goto err1;
  nodelist_push(t->nodes,&default_node);
  t->items=itemlist_alloc(0,NULL);
  if(t->items==NULL) goto err2;
  t->free_value_func=free_value_func;
  return t;
  err2: nodelist_free(t->nodes);
  err1: free(t);
  err0: return NULL;
}

static void free_items(tst t)
{
  if(t->items==NULL) return;
  item *last=itemlist_back(t->items);
  if(last)
    {
      item *i;
      for(i=itemlist_front(t->items);i<=last;i++)
        {
          if(i->key) free(i->key);
          if(t->free_value_func&&(i->value!=NULL))
            t->free_value_func(i->value);
        }
    }
  itemlist_free(t->items);
  t->items=NULL;
}

void tst_free(tst t)
{
  if(t==NULL) return;
  free_items(t);
  nodelist_free(t->nodes);
  free(t);
}

static size_t new_node(nodelist nodes,ucs4_t c)
{
  if(!nodelist_push(nodes,&default_node)) return 0;
  nodelist_back(nodes)->spltchr=c;
  return nodelist_size(nodes)-1;
}

static node *insert_key(tst t,const uint8_t *key)
{
  size_t n=0;
  size_t k;
  ucs4_t sc,c;
  const uint8_t *s=u8_next(&c,key);
  if(s==NULL) return NULL;
  c=uc_tolower(c);
  if(nodelist_front(t->nodes)->spltchr=='\0') nodelist_front(t->nodes)->spltchr=c;
  while(1)
    {
      sc=nodelist_at(t->nodes,n)->spltchr;
      if(c<sc)
        {
          if(nodelist_at(t->nodes,n)->left==0)
            {
              k=new_node(t->nodes,c);
              if(k==0) return NULL;
              nodelist_at(t->nodes,n)->left=k;
            }
          n=nodelist_at(t->nodes,n)->left;
        }
      else if(c==sc)
        {
          if(c=='\0') break;
          else
            {
              s=u8_next(&c,s);
              if(s==NULL) c='\0';
              else c=uc_tolower(c);
              if(nodelist_at(t->nodes,n)->mid==0)
            {
              k=new_node(t->nodes,c);
              if(k==0) return NULL;
              nodelist_at(t->nodes,n)->mid=k;
            }
          n=nodelist_at(t->nodes,n)->mid;
            }
        }
      else
        {
          if(nodelist_at(t->nodes,n)->right==0)
            {
              k=new_node(t->nodes,c);
              if(k==0) return NULL;
              nodelist_at(t->nodes,n)->right=k;
            }
          n=nodelist_at(t->nodes,n)->right;
        }
    }
  return nodelist_at(t->nodes,n);
}

static int simple_caseless_compare(const uint8_t *s1,const uint8_t *s2)
{
  ucs4_t c1,c2;
  int d=0;
  if((s1[0]!='\0')||(s2[0]!='\0'))
    {
      do
        {
          s1=u8_next(&c1,s1);
          s2=u8_next(&c2,s2);
          if((s1==NULL)&&(s2==NULL)) break;
          if(s1!=NULL) d=uc_tolower(c1);
          if(s2!=NULL) d-=uc_tolower(c2);
        }
      while(d==0);
    }
  return d;
}

static int item_cmp(const void *p1,const void *p2)
{
  const uint8_t *s1=((const item*)p1)->key;
  const uint8_t *s2=((const item*)p2)->key;
  int r=simple_caseless_compare(s1,s2);
  return (r!=0)?r:(((const item*)p1)->index-((const item*)p2)->index);
}

int tst_add(tst t,const uint8_t *key,void *value)
{
  if(t==NULL) return 0;
  item i;
  if(itemlist_size(t->items)==0xfffff) return 0;
  i.value=value;
  i.index=itemlist_size(t->items);
  i.key=u8_strdup(key);
  if(i.key==NULL) return 0;
  if(!itemlist_push(t->items,&i))
    {
      free(i.key);
      return 0;
    }
  return 1;
}

vector_t(size_t,argstack)

void tst_build(tst t)
{
  if(t==NULL) return;
  if(t->items==NULL) return;
  if(itemlist_size(t->items)==0) return;
  qsort(itemlist_data(t->items),itemlist_size(t->items),sizeof(item),item_cmp);
  item *p=itemlist_front(t->items);
  p->index=0;
  const uint8_t *key=p->key;
  size_t i,k,m,n;
  n=itemlist_size(t->items);
  k=1;
  for(i=1;i<n;i++)
    {
      p=itemlist_at(t->items,i);
      if(simple_caseless_compare(key,p->key)==0)
        p->index=(p-1)->index+1;
      else
        {
          p->index=0;
          key=p->key;
          k++;
        }
    }
  argstack v=argstack_alloc(k,NULL);
  if(v==NULL) return;
  for(i=0;i<n;i++)
    {
      if(itemlist_at(t->items,i)->index==0)
        argstack_push(v,&i);
    }
  argstack s=argstack_alloc(2,NULL);
  if(s==NULL)
    {
      argstack_free(v);
      return;
    }
  node *tn;
  i=0;
  argstack_push(s,&i);
  i=argstack_size(v);
  argstack_push(s,&i);
  while(argstack_size(s)>0)
    {
      n=*argstack_back(s);
      argstack_pop(s);
      k=*argstack_back(s);
      argstack_pop(s);
      if(n>0)
        {
          m=n/2;
          i=*argstack_at(v,k+m);
          p=itemlist_at(t->items,i);
          tn=insert_key(t,p->key);
          if(tn==NULL) break;
          tn->mid=i;
          if(!argstack_reserve(s,argstack_size(s)+4)) break;
          i=k+m+1;
          argstack_push(s,&i);
          i=n-m-1;
          argstack_push(s,&i);
          argstack_push(s,&k);
          argstack_push(s,&m);
        }
    }
  argstack_free(s);
  argstack_free(v);
}

void tst_enum_prefixes(tst t,const uint8_t *key,tst_enum_callback f,void *d)
{
  if(nodelist_front(t->nodes)->spltchr=='\0') return;
  ucs4_t c,sc;
  const uint8_t *s=u8_next(&c,key);
  if(s==NULL) return;
  c=uc_tolower(c);
  size_t i;
  size_t k=itemlist_size(t->items);
  const item*p;
  size_t l=0;
  size_t n=0;
  argstack r=argstack_alloc(2*u8_mbsnlen(s,u8_strlen(s)),NULL);
  if(r==NULL) return;
  do
    {
      sc=nodelist_at(t->nodes,n)->spltchr;
      if(c<sc) n=nodelist_at(t->nodes,n)->left;
      else if(c==sc)
        {
          if(c=='\0') break;
          l=s-key;
          n=nodelist_at(t->nodes,n)->mid;
          argstack_push(r,&n);
          argstack_push(r,&l);
          s=u8_next(&c,s);
          if(s==NULL) c='\0';
          else c=uc_tolower(c);
        }
      else n=nodelist_at(t->nodes,n)->right;
    }
  while(n>0);
  while(argstack_size(r)>0)
    {
      l=*argstack_back(r);
      argstack_pop(r);
      n=*argstack_back(r);
      argstack_pop(r);
      while(n)
        {
          if(nodelist_at(t->nodes,n)->spltchr=='\0')
            {
              i=nodelist_at(t->nodes,n)->mid;
              p=itemlist_at(t->items,i);
              do
                {
                  if(!f(p->key,l,p->value,d))
                    {
                      argstack_free(r);
                      return;
                    }
                  i++;
                  if(i==k) break;
                  p=itemlist_at(t->items,i);
                }
              while((p->index>0));
              break;
            }
          else
            n=nodelist_at(t->nodes,n)->left;
        }
    }
  argstack_free(r);
}
