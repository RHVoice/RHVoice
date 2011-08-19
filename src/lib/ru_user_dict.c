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
#include <stdio.h>
#include <unistr.h>
#include <unictype.h>
#include <pcre.h>
#include "tst.h"
#include "lib.h"
#include "ustring.h"
#include "vector.h"
#include "io_utils.h"

#define max_pron_len 80

static const uint8_t space_delims[]={' ','\t','\0'};

typedef struct
{
  pcre *prefix_regex;
  pcre *suffix_regex;
  int ignore_case;
  uint8_t *pron;
} rule;

static void free_rule(void *p)
{
  rule *r=(rule*)p;
  if(r==NULL) return;
  if(r->pron) free(r->pron);
  if(r->prefix_regex) pcre_free(r->prefix_regex);
  if(r->suffix_regex) pcre_free(r->suffix_regex);
  free(r);
}

typedef struct
{
  uint8_t *name;
  tst words;
} user_dict;

static int init_user_dict(user_dict *dict)
{
  dict->words=tst_create(free_rule);
  if(dict->words==NULL) return 0;
  dict->name=NULL;
  return 1;
}

static void free_user_dict(user_dict *dict)
{
  if(dict==NULL) return;
  if(dict->name) free(dict->name);
  tst_free(dict->words);
}

vector_t(user_dict,dictlist)

static dictlist dicts=NULL;

static int check_user_pron_value(const uint8_t *p)
{
  ucs4_t c;
  unsigned int cs;
  const uint8_t *s=p+u8_strspn(p,space_delims);
  if(s[0]=='\0') return 0;
  if(u8_mbsnlen(s,u8_strlen(s))>max_pron_len) return 0;
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
  const uint8_t *s1=s;
  const uint8_t *s2;
  ucs4_t c;
  size_t n=1;
  while((s2=u8_next(&c,s1)))
    {
      if((c!=' ')&&(c!='\t'))
        {
          n+=(s2-s1);
          if((s2[0]=='\0')||(s2[0]==' ')||(s2[0]=='\t')) n++;
        }
      s1=s2;
    }
  uint8_t *r=(uint8_t*)malloc(n);
  if(r==NULL) return NULL;
  size_t i=0;
  size_t l=0;
  s1=s;
  while((s2=u8_next(&c,s1)))
    {
      if((c!=' ')&&(c!='\t'))
        {
          l=s2-s1;
          u8_cpy(&r[i],s1,l);
          i+=l;
          if((s2[0]=='\0')||(s2[0]==' ')||(s2[0]=='\t')) r[i++]='\0';
        }
      s1=s2;
    }
  r[n-1]='\0';
  return r;
}

static void add_word(user_dict *dict,const uint8_t *key,const uint8_t *value)
{
  const char *errptr=NULL;
  int erroffset=0;
  if(value==NULL) return;
  if(!check_user_pron_value(value)) return;
  rule *r=malloc(sizeof(rule));
  if(r==NULL) return;
  r->prefix_regex=NULL;
  r->suffix_regex=NULL;
  r->pron=copy_user_pron_value(value);
  if(r->pron==NULL) goto err0;
  r->ignore_case=1;
  uint8_t *s=u8_strdup(key);
  if(s==NULL) goto err1;
  uint8_t *word=NULL;
  uint8_t *part1=NULL;
  uint8_t *part2=NULL;
  uint8_t *part3=NULL;
  uint8_t *p=NULL;
  part1=u8_strtok(s,space_delims,&p);
  if(part1)
    {
      part2=u8_strtok(NULL,space_delims,&p);
      if(part2)
        {
          part3=u8_strtok(NULL,space_delims,&p);
        }
    }
  if(part3)
    {
      r->prefix_regex=pcre_compile((const char*)part1,PCRE_UTF8|PCRE_UCP,&errptr,&erroffset,NULL);
      if(r->prefix_regex)
        {
          r->suffix_regex=pcre_compile((const char*)part3,PCRE_UTF8|PCRE_UCP,&errptr,&erroffset,NULL);
          if(r->suffix_regex)
            {
              word=part2;
            }
          else
            {
              pcre_free(r->prefix_regex);
              r->prefix_regex=NULL;
            }
        }
    }
  else if(part2==NULL)
    word=part1;
  if(!word) goto err2;
  size_t l=u8_strlen(word);
  if((l>2)&&(word[l-2]=='/'))
    {
      if(word[l-1]=='c')
        {
          r->ignore_case=0;
          word[l-2]='\0';
        }
      else if(word[l-1]=='i')
        word[l-2]='\0';
    }
  if(!tst_add(dict->words,word,r)) goto err3;
  free(s);
  return;
  err3: if(r->suffix_regex) {pcre_free(r->prefix_regex);pcre_free(r->suffix_regex);}
  err2: free(s);
  err1: free(r->pron);
 err0: free(r);
}

static int user_dict_callback(const uint8_t *section,const uint8_t *key,const uint8_t *value,void *user_data)
{
  user_dict *dict=(user_dict*)user_data;
  if(section==NULL)
    {
      if(value!=NULL)
        {
          if(u8_strcmp(key,(const uint8_t*)"name")==0)
            {
              if(dict->name!=NULL) free(dict->name);
              dict->name=u8_strdup(value);
            }
        }
    }
  else if(u8_strcmp(section,(const uint8_t*)"dict")==0)
      add_word(dict,key,value);
  return 1;
}

static int update_user_dict(const char *path,void *data)
{
  user_dict *dict=(user_dict*)data;
  if(dict!=NULL)
    {
      parse_config(path,user_dict_callback,dict);
    }
  else
    {
      if(!dictlist_reserve(dicts,dictlist_size(dicts)+1)) return 0;
      user_dict tmp;
      if(!init_user_dict(&tmp)) return 0;
      dictlist_push(dicts,&tmp);
      dict=dictlist_back(dicts);
      parse_config(path,user_dict_callback,dict);
      if(dict->name==NULL)
        {
          if(dict->name==malloc(8))
            snprintf((char*)(&dict->name),8,"%u",(unsigned int)dictlist_size(dicts)+builtin_variant_count-1);
        }
      tst_build(dict->words);
    }
  return 1;
}

void load_user_dicts(const char *path)
{
  if(path==NULL) return;
  dicts=dictlist_alloc(1,free_user_dict);
  if(dicts==NULL) return;
  user_dict dict;
  if(!init_user_dict(&dict)) return;
  dictlist_push(dicts,&dict);
  char *subpath=path_append(path,"dicts");
  if(subpath==NULL) return;
  for_each_file_in_dir(subpath,update_user_dict,dictlist_back(dicts));
  tst_build(dictlist_back(dicts)->words);
  free(subpath);
  subpath=path_append(path,"variants");
  if(subpath==NULL) return;
  for_each_file_in_dir(subpath,update_user_dict,NULL);
  free(subpath);
}

void free_user_dicts()
{
  if(dicts==NULL) return;
  dictlist_free(dicts);
  dicts=NULL;
}

int get_user_dicts_count()
{
  if(dicts==NULL) return 0;
  return dictlist_size(dicts);
}

const uint8_t *user_dict_get_name(int i)
{
  if(i>=get_user_dicts_count()) return NULL;
  return dictlist_at(dicts,i)->name;
}

typedef struct
{
  const uint8_t *word;
  size_t start;
  size_t len;
  const uint8_t *pron;
} lookup_result;

static int enum_callback(const uint8_t *key,size_t len,void *value,void *user_data)
{
  rule *r=(rule*)value;
  lookup_result *p=(lookup_result*)user_data;
  ucs4_t c='\0';
  if(r->prefix_regex==NULL)
    {
      if(p->start>0)
        {
          u8_prev(&c,p->word+p->start,p->word);
          if(!uc_is_property(c,UC_PROPERTY_PUNCTUATION)) return 1;
        }
      if(p->word[p->start+len]!='\0')
        {
          u8_next(&c,p->word+p->start+len);
          if(!uc_is_property(c,UC_PROPERTY_PUNCTUATION)) return 1;
        }
    }
  if((r->ignore_case==0)&&(u8_cmp(key,p->word+p->start,len)!=0)) return 1;
  if(r->prefix_regex!=NULL)
    {
      int o[30];
      if(pcre_exec(r->prefix_regex,NULL,(const char*)p->word,p->start,0,0,o,30)<0) return 1;
      if(pcre_exec(r->suffix_regex,NULL,(const char*)p->word+p->start+len,p->len-p->start-len,0,0,o,30)<0) return 1;
    }
  p->len=len;
  p->pron=r->pron;
  return 0;
}

const uint8_t *user_dict_lookup(int i,const uint8_t *word,size_t *offset)
{
  if(dicts==NULL) return NULL;
  if(i>=dictlist_size(dicts)) return NULL;
  user_dict *dict=dictlist_at(dicts,i);
  lookup_result res;
  res.word=word;
  res.start=*offset;
  res.len=u8_strlen(word);
  res.pron=NULL;
  tst_enum_prefixes(dict->words,word+*offset,enum_callback,&res);
  if(res.pron)
    {
      *offset+=res.len;
      return res.pron;
    }
  else
    return NULL;
}
