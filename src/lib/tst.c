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

#include <stdint.h>
#include <stdlib.h>
#include <search.h>
#include <unistr.h>
#include "tst.h"
#include "vector.h"

typedef struct
{
  ucs4_t spltchr;
  size_t left,right;
  uintptr_t mid;
} node;

vector_t(node,nodelist)

static node default_node={'\0',0,0,0};

typedef struct
{
  uint8_t *key;
  void *value;
  size_t index;
} item;

vector_t(item,itemlist)

struct tst_s
{
  nodelist nodes;
  itemlist items;
  int store_values;
  tst_free_value_func free_value_func;
};

tst tst_create(int store_values,tst_free_value_func free_value_func)
{
  tst t=(tst)malloc(sizeof(struct tst_s));
  if(t==NULL) goto err0;
  t->nodes=nodelist_alloc(1,NULL);
  if(t->nodes==NULL) goto err1;
  nodelist_push(t->nodes,&default_node);
  t->items=itemlist_alloc(0,NULL);
  if(t->items==NULL) goto err2;
  t->store_values=store_values;
  t->free_value_func=store_values?free_value_func:NULL;
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
          if(t->store_values&&t->free_value_func&&(i->value!=NULL))
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
  if(t->store_values&&t->free_value_func)
    {
      node *last=nodelist_back(t->nodes);
      node *n;
      for(n=nodelist_front(t->nodes)+1;n<=last;n++)
        {
          if((n->spltchr=='\0')&&(n->mid!=0)) t->free_value_func((void*)n->mid);
        }
    }
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

static int item_cmp(const void *p1,const void *p2)
{
  const item *i1=(const item*)p1;
  const item *i2=(const item*)p2;
  int r=u8_strcmp(i1->key,i2->key);
  return (r!=0)?r:(i1->index-i2->index);
}

int tst_add(tst t,const uint8_t *key,void *value)
{
  if(t==NULL) return 0;
  item i;
  i.value=(t->store_values)?value:NULL;
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
  argstack s=argstack_alloc(2,NULL);
  if(s==NULL) return;
  item *p;
  size_t i,k,m,n;
  node *tn;
  i=0;
  argstack_push(s,&i);
  i=itemlist_size(t->items);
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
          p=itemlist_at(t->items,k+m);
          tn=insert_key(t,p->key);
          if(tn==NULL) break;
          if(t->store_values&&(p->index>=tn->mid))
            {
              tn->mid=(uintptr_t)p->value;
              p->value=NULL;
            }
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
  free_items(t);
}

int tst_search8(tst t,const uint8_t *key,void **pvalue)
{
  if(nodelist_front(t->nodes)->spltchr=='\0') return 0;
  ucs4_t c,sc;
  const uint8_t *s=u8_next(&c,key);
  if(s==NULL) return 0;
  size_t n=0;
  do
    {
      sc=nodelist_at(t->nodes,n)->spltchr;
      if(c<sc) n=nodelist_at(t->nodes,n)->left;
      else if(c==sc)
        {
          if(c=='\0')
            {
              if(t->store_values&&(pvalue!=NULL))
                *pvalue=(void*)nodelist_at(t->nodes,n)->mid;
              return 1;
            }
          n=nodelist_at(t->nodes,n)->mid;
          s=u8_next(&c,s);
          if(s==NULL) c='\0';
        }
      else n=nodelist_at(t->nodes,n)->right;
    }
  while(n>0);
  return 0;
}

int tst_search32(tst t,const uint32_t *key,void **pvalue)
{
  if(nodelist_front(t->nodes)->spltchr=='\0') return 0;
  ucs4_t c,sc;
  const uint32_t *s=u32_next(&c,key);
  if(s==NULL) return 0;
  size_t n=0;
  do
    {
      sc=nodelist_at(t->nodes,n)->spltchr;
      if(c<sc) n=nodelist_at(t->nodes,n)->left;
      else if(c==sc)
        {
          if(c=='\0')
            {
              if(t->store_values&&(pvalue!=NULL))
                *pvalue=(void*)nodelist_at(t->nodes,n)->mid;
              return 1;
            }
          n=nodelist_at(t->nodes,n)->mid;
          s=u32_next(&c,s);
          if(s==NULL) c='\0';
        }
      else n=nodelist_at(t->nodes,n)->right;
    }
  while(n>0);
  return 0;
}
