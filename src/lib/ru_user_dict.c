/* Copyright (C) 2010, 2011  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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
#include <unistr.h>
#include "tree.h"
#include "lib.h"
#include "ustring.h"

static const uint8_t space_delims[]={' ','\t','\0'};

struct word_node
{
  uint8_t *word;
  uint8_t *pron;
  RB_ENTRY(word_node) links;
};

RB_HEAD(word_tree,word_node);

static void delete_word_node(struct word_node *el)
{
  free(el->word);
  free(el->pron);
  free(el);
}

static int word_cmp(const struct word_node *el1,const struct word_node *el2)
{
  return u8_strcmp(el1->word,el2->word);
}

RB_GENERATE_STATIC(word_tree,word_node,links,word_cmp)

struct user_dict_s
{
  struct word_tree words;
};

user_dict user_dict_create()
{
  user_dict dict=(user_dict)malloc(sizeof(struct user_dict_s));
  if(dict==NULL) return NULL;
  RB_INIT(&dict->words);
  return dict;
}

void user_dict_free(user_dict dict)
{
  if(dict==NULL) return;
  struct word_node *el,*tmp;
  el=RB_MIN(word_tree,&dict->words);
  while(el)
    {
      tmp=RB_NEXT(word_tree,&dict->words,el);
      RB_REMOVE(word_tree,&dict->words,el);
      delete_word_node(el);
      el=tmp;
    }
  free(dict);
}

static int check_user_pron_value(const uint8_t *p)
{
  ucs4_t c;
  unsigned int cs;
  const uint8_t *s=p+u8_strspn(p,space_delims);
  if(s[0]=='\0') return 0;
  while((s=u8_next(&c,s)))
    {
      cs=classify_character(c);
      if(!(((cs&cs_ru)&&(cs&cs_ll))||(c=='+')||(c==' ')||(c=='\t'))) return 0;
    }
  return 1;
}

static uint8_t *copy_user_pron_value(const uint8_t *p)
{
  const uint8_t *s=p+u8_strspn(p,space_delims);
  size_t l=u8_strlen(s);
  size_t n=1;
  size_t i;
  for(i=0;i<l;i++)
    {
      if((s[i]!=' ')&&(s[i]!='\t'))
        {
          n++;
          if((s[i+1]=='\0')||(s[i+1]==' ')||(s[i+1]=='\t')) n++;
        }
    }
  uint8_t *r=(uint8_t*)malloc(n);
  if(r==NULL) return NULL;
  size_t j=0;
  for(i=0;i<l;i++)
    {
      if((s[i]!=' ')&&(s[i]!='\t'))
        {
          r[j++]=s[i];
          if((s[i+1]=='\0')||(s[i+1]==' ')||(s[i+1]=='\t')) r[j++]='\0';
        }
    }
  r[n-1]='\0';
  return r;
}

static void add_word(user_dict dict,const uint8_t *key,const uint8_t *value)
{
  if(value==NULL) return;
  if(!check_user_pron_value(value)) return;
  uint8_t *pron=copy_user_pron_value(value);
  if(pron==NULL) return;
  struct word_node find;
  find.word=(uint8_t*)key;
  struct word_node *node=RB_FIND(word_tree,&dict->words,&find);
  if(node!=NULL)
    {
      free(node->pron);
      node->pron=pron;
    }
  else
    {
      node=(struct word_node*)malloc(sizeof(struct word_node));
      if(node==NULL)
        {
          free(pron);
          return;
        }
      node->word=u8_strdup(key);
      if(node->word==NULL)
        {
          free(pron);
          free(node);
          return;
        }
      node->pron=pron;
      RB_INSERT(word_tree,&dict->words,node);
    }
}

static int dict_callback(const uint8_t *section,const uint8_t *key,const uint8_t *value,void *user_data)
{
  user_dict dict=(user_dict)user_data;
  if(u8_strcmp(section,(const uint8_t*)"main")==0)
      add_word(dict,key,value);
  return 1;
}

void user_dict_update(user_dict dict,const char *path)
{
  if(dict==NULL) return;
  parse_config(path,dict_callback,dict);
}

const uint8_t* user_dict_lookup(const user_dict dict,const uint8_t* word)
{
  if(dict==NULL) return NULL;
  struct word_node find;
  find.word=(uint8_t*)word;
  struct word_node *found=RB_FIND(word_tree,&dict->words,&find);
  return (found==NULL)?NULL:(found->pron);
}
