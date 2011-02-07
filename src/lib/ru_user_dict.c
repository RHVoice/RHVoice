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
#include <unicase.h>
#include "lib.h"

static int ru_user_dict_cmp(const void *p1,const void *p2)
{
  return u8_strcmp(**((const uint8_t***)p1),**((const uint8_t***)p2));
}

static cst_val *load_entries(const char *path)
{
  cst_tokenstream *ts_lines;
  ucs4_t ru_a,ru_ya,ru_yo,c;
  cst_val *entries=NULL;
  cst_val *entry;
  const char *line;
  uint8_t *lcline=NULL;
  size_t len=0;
  const uint8_t *word;
  uint8_t *pron;
  const uint8_t *entry_delims=(const uint8_t*)" \t";
  const uint8_t *word_delims=(const uint8_t*)"-";
  uint8_t *ptr;
  int only_valid_chars;
  const uint8_t *s;
  u8_strmbtouc(&ru_a,(const uint8_t*)"а");
  u8_strmbtouc(&ru_ya,(const uint8_t*)"я");
  u8_strmbtouc(&ru_yo,(const uint8_t*)"ё");
  ts_lines=ts_open(path,"\r\n","","","");
  if(ts_lines==NULL)
    return NULL;
  while(!ts_eof(ts_lines))
    {
      line=ts_get(ts_lines);
      len=u8_strlen((const uint8_t*)line);
      if(len==0)
        break;
      lcline=u8_tolower((const uint8_t*)line,len,"ru",NULL,NULL,&len);
      if(lcline==NULL)
        continue;
      lcline=(uint8_t*)realloc(lcline,len+1);
      lcline[len]='\0';
      ptr=NULL;
      word=u8_strtok(lcline,entry_delims,&ptr);
      if((word!=NULL)&&((entries==NULL)||!val_assoc_string((const char*)word,entries)))
        {
          pron=u8_strtok(NULL,entry_delims,&ptr);
          if(pron!=NULL)
            {
              only_valid_chars=TRUE;
              for(s=u8_next(&c,pron);s;s=u8_next(&c,s))
                {
                  if(!((c>=ru_a&&c<=ru_ya)||(c==ru_yo)||(c=='+')||(c=='-')))
                    {
                      only_valid_chars=FALSE;
                      break;
                    }
                }
              if(only_valid_chars)
                {
                  entry=cons_val(string_val((const char*)word),NULL);
                  ptr=NULL;
                  for(word=u8_strtok(pron,word_delims,&ptr);word;word=u8_strtok(NULL,word_delims,&ptr))
                    {
                      entry=cons_val(string_val((const char*)word),entry);
                    }
                  if(val_length(entry)>=2)
                    entries=cons_val(val_reverse(entry),entries);
                  else
                    delete_val(entry);
                }
            }
        }
      free(lcline);
      lcline=NULL;
    }
  ts_close(ts_lines);
  return val_reverse(entries);
}

ru_user_dict *ru_user_dict_load(const char *path)
{
  ru_user_dict *dict;
  char ***p1,**p2;
  const cst_val *l1,*l2;
  cst_val *entries=load_entries(path);
  if(entries==NULL)
    return NULL;
  dict=(ru_user_dict*)malloc(sizeof(ru_user_dict));
  if(dict==NULL)
    {
      delete_val(entries);
      return NULL;
    }
  dict->size=val_length(entries);
  dict->entries=(char***)calloc(dict->size,sizeof(char**));
  if(dict->entries==NULL)
    {
      delete_val(entries);
      free(dict);
      return NULL;
    }
  for(l1=entries,p1=dict->entries;l1;l1=val_cdr(l1),p1++)
    {
      *p1=(char**)calloc(val_length(val_car(l1))+1,sizeof(char*));
      if(*p1==NULL)
        {
          delete_val(entries);
          ru_user_dict_free(dict);
          return NULL;
        }
      for(l2=val_car(l1),p2=*p1;l2;l2=val_cdr(l2),p2++)
        {
          *p2=(char*)u8_strdup((const uint8_t*)val_string(val_car(l2)));
          if(*p2==NULL)
            {
              delete_val(entries);
              ru_user_dict_free(dict);
              return NULL;
            }
        }
    }
  delete_val(entries);
  qsort(dict->entries,dict->size,sizeof(char**),ru_user_dict_cmp);
  return dict;
}

void ru_user_dict_free(ru_user_dict *dict)
{
  int i;
  char **p;
  if(dict==NULL)
    return;
  if(dict->entries==NULL)
    return;
  for(i=0;i<dict->size;i++)
    {
      if(dict->entries[i]==NULL)
        break;
      for(p=dict->entries[i];*p!=NULL;p++)
        {
          free(*p);
        }
      free(dict->entries[i]);
    }
  free(dict->entries);
  free(dict);
}

const char **ru_user_dict_lookup(const ru_user_dict *dict,const char *word)
{
  const char *p1=word;
  const char **p2=&p1;
  const char ***p=bsearch(&p2,dict->entries,dict->size,sizeof(char**),ru_user_dict_cmp);
  if(p==NULL)
    return NULL;
  else
    return *p;
}
