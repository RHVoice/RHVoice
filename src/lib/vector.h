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

#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>
#include <string.h>

#define vector_t(type,name)\
typedef void(*name##_free_func)(type*);\
struct name##_s\
{\
  type* d;\
  size_t n,m;\
  name##_free_func f;\
};\
typedef struct name##_s *name;\
\
static name name##_reserve(name v,size_t r)\
{\
  if(v==NULL) {return NULL;}\
  if(v->m>=r) {return v;}\
  size_t m0=v->m*2;\
  size_t m=(m0>r)?m0:r;\
  size_t s=m*sizeof(type);\
  type* t=(type*)((v->d==NULL)?malloc(s):realloc(v->d,s));\
  if(t==NULL) {return NULL;}\
  v->d=t;\
  v->m=m;\
  return v;\
}\
\
static name name##_alloc(size_t min_elements,name##_free_func free_func)\
{\
  name v=(name)malloc(sizeof(struct name##_s));\
  if(v==NULL) {return NULL;}\
  v->f=free_func;\
  v->d=NULL;\
  v->n=0;\
  v->m=0;\
  if(!name##_reserve(v,min_elements))\
  {\
    free(v);\
    return NULL;\
  }\
  return v;\
}\
\
static void name##_erase(name v,size_t p,size_t c)\
{\
  size_t i;\
  if(v==NULL) {return;}\
  if(p>=v->n) {return;}\
  size_t n=(c==0)?v->n:(p+c);\
  if(n>v->n) {n=v->n;}\
  size_t r=v->n-n;\
  if(v->f!=NULL)\
    {\
      for(i=p;i<n;i++)\
        {\
          v->f(&v->d[i]);\
        }\
    }\
  if(r>0) {memmove(&v->d[p],&v->d[n],r*sizeof(type));}\
  v->n=p+r;\
}\
\
static void name##_clear(name v)\
{\
  name##_erase(v,0,0);\
}\
\
static void name##_free(name v)\
{\
  if(v==NULL) {return;}\
  if(v->n>0) {name##_clear(v);}\
  if(v->d!=NULL) {free(v->d);}\
  free(v);\
}\
\
static size_t name##_size(name v)\
{\
  return (v?v->n:0);\
}\
\
static type* name##_data(name v)\
{\
  return (v?v->d:NULL);\
}\
\
static size_t name##_element_size(name v)\
{\
  return sizeof(type);\
}\
\
static name name##_append(name v,type* p,size_t c)\
{\
  if((v==NULL)||(p==NULL)||(c==0)) {return NULL;}\
  if(!name##_reserve(v,v->n+c)) {return NULL;}\
  if(c==1)\
    {v->d[v->n]=*p;}\
  else\
    {memcpy(&v->d[v->n],p,c*sizeof(type));}\
  v->n+=c;\
  return v;\
}\
\
static name name##_push(name v,type* p)\
{\
  return name##_append(v,p,1);\
}\
\
static void name##_pop(name v)\
{\
  if(v==NULL) {return;}\
  if(v->n==0) {return;}\
  if(v->f) {v->f(&v->d[v->n-1]);}\
  v->n--;\
}\
\
static type* name##_back(name v)\
{\
  if(v==NULL) {return NULL;}\
  if(v->n==0) {return NULL;}\
  return &v->d[v->n-1];\
}\
\
static type* name##_front(name v)\
{\
  if(v==NULL) {return NULL;}\
  if(v->n==0) {return NULL;}\
  return &v->d[0];\
}\
\
static type* name##_at(name v,size_t i)                \
{\
  if(v==NULL) {return NULL;}\
  if(i>=v->n) {return NULL;}\
  return &v->d[i];\
}\
\
static name name##_resize(name v,size_t s,type *dv)\
{\
  if(v==NULL) {return NULL;}\
  if(s==v->n) {return v;}\
  if(s<v->n) {name##_erase(v,s,0);return v;}\
  if(dv==NULL) {return NULL;}\
  if(!name##_reserve(v,s)) {return NULL;}\
  size_t i;\
  type val=*dv;\
  for(i=v->n;i<s;i++)\
    {\
      v->d[i]=val;\
    }\
  v->n=s;\
  return v;\
}
#endif
