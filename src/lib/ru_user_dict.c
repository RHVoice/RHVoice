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
#include "tst.h"
#include "lib.h"
#include "ustring.h"

static const uint8_t space_delims[]={' ','\t','\0'};

static void free_pron(void *p)
{
  free(p);
}

struct user_dict_s
{
  tst words;
  ucs4_t stress_marker;
  int mark_next_vowel_as_stressed;
};

user_dict user_dict_create()
{
  user_dict dict=(user_dict)malloc(sizeof(struct user_dict_s));
  if(dict==NULL) goto err0;
  dict->words=tst_create(1,free_pron);
  if(dict->words==NULL) goto err1;
  dict->stress_marker='+';
  dict->mark_next_vowel_as_stressed=1;
  return dict;
  err1: free(dict);
  err0: return NULL;
}

void user_dict_free(user_dict dict)
{
  if(dict==NULL) return;
  tst_free(dict->words);
  free(dict);
}

static int check_user_pron_value(const user_dict d,const uint8_t *p)
{
  ucs4_t c;
  ucs4_t pc='\0';
  unsigned int cs;
  unsigned int pcs=0;
  const uint8_t *s=p+u8_strspn(p,space_delims);
  if(s[0]=='\0') return 0;
  while((s=u8_next(&c,s)))
    {
      cs=classify_character(c);
      if(!(((cs&cs_ru)&&(cs&cs_ll))||(c==d->stress_marker)||(c==' ')||(c=='\t'))) return 0;
      if(d->mark_next_vowel_as_stressed)
        {
          if((pc==d->stress_marker)&&!(cs&cs_lv)) return 0;
        }
      else
        {
          if((c==d->stress_marker)&&!(pcs&cs_lv)) return 0;
        }
      pc=c;
      pcs=cs;
    }
  return 1;
}

static uint8_t *copy_user_pron_value(user_dict d,const uint8_t *p)
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
          if(c==d->stress_marker) n++;
          else n+=(s2-s1);
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
          if(c==d->stress_marker)
            {
              if(d->mark_next_vowel_as_stressed)
                r[i++]='+';
              else
                {
                  u8_move(&r[i-l+1],&r[i-l],l);
                  r[i-l]='+';
                  i++;
                }
            }
          else
            {
              l=s2-s1;
              u8_cpy(&r[i],s1,l);
              i+=l;
            }
          if((s2[0]=='\0')||(s2[0]==' ')||(s2[0]=='\t')) r[i++]='\0';
        }
      s1=s2;
    }
  r[n-1]='\0';
  return r;
}

static void add_word(user_dict dict,const uint8_t *key,const uint8_t *value)
{
  if(value==NULL) return;
  if(!check_user_pron_value(dict,value)) return;
  uint8_t *pron=copy_user_pron_value(dict,value);
  if(pron==NULL) return;
  if(!tst_add(dict->words,key,pron)) free(pron);
}

static int dict_callback(const uint8_t *section,const uint8_t *key,const uint8_t *value,void *user_data)
{
  user_dict dict=(user_dict)user_data;
  if(section==NULL)
    {
      if(value!=NULL)
        {
          if(u8_strcmp(key,(const uint8_t*)"stress_marker")==0)
            {
              u8_next(&dict->stress_marker,value);
            }
          else if(u8_strcmp(key,(const uint8_t*)"mark_next_vowel_as_stressed")==0)
            {
              if(strchr("tTyY1",(char)value[0]))
                dict->mark_next_vowel_as_stressed=1;
              else if(strchr("fFnN0",(char)value[0]))
                dict->mark_next_vowel_as_stressed=0;
            }
        }
    }
  else if(u8_strcmp(section,(const uint8_t*)"dict")==0)
      add_word(dict,key,value);
  return 1;
}

int user_dict_update(const char *path,void *data)
{
  user_dict dict=(user_dict)data;
  if(dict==NULL) return 0;
  parse_config(path,dict_callback,dict);
  dict->stress_marker='+';
  dict->mark_next_vowel_as_stressed=1;
  return 1;
}

void user_dict_build(user_dict dict)
{
  if(dict==NULL) return;
  tst_build(dict->words);
}

const uint8_t* user_dict_lookup(const user_dict dict,const uint8_t* word)
{
  if(dict==NULL) return NULL;
  uint8_t *p=NULL;
  if(tst_search(dict->words,word,(void**)&p))
    return p;
  else
    return NULL;
}
