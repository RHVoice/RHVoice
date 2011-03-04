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

#include <search.h>
#include <unistr.h>
#include <unicase.h>
#include "lib.h"
#include "io_utils.h"
#include "vector.h"
#include "ustring.h"

typedef struct {
  uint8_t *word;
  uint8_t *pron;
} entry;

vector_t(entry,userdict)

static void entry_free(entry *e)
{
  free(e->word);
  free(e->pron);
}

static int cmp(const void *p1,const void *p2)
{
  const entry *e1=(const entry*)p1;
  const entry *e2=(const entry*)p2;
  return u8_strcmp(e1->word,e2->word);
}

static int load_entry(ustring32_t line,entry *e)
{
  ustring32_push(line,'\0');
  size_t n=ustring32_length(line)-1;
  unsigned int cs;
  size_t i;
  ucs4_t c;
  for(i=0;i<n;i++)
    {
      c=ustring32_at(line,i);
      cs=classify_character(c);
      if(!((cs&cs_l)||(c==' ')||(c=='\t')||(c=='+')||(c=='-')))
        return 0;
    }
  uint32_t delims[]={' ','\t','\0'};
  uint32_t *word,*pron,*ptr;
  word=u32_strtok(ustring32_str(line),delims,&ptr);
  if(word==NULL) return 0;
  pron=u32_strtok(NULL,delims,&ptr);
  if(pron==NULL) return 0;
  e->word=u32_to_u8(word,u32_strlen(word)+1,NULL,&n);
  if(e->word==NULL) return 0;
  n=u32_strlen(pron);
  c='\0';
  for(i=0;i<n;i++)
    {
      if((pron[i]=='-')&&(classify_character(c)&cs_l))
        pron[i]='\0';
      c=pron[i];
    }
  n+=2;
  pron[n-1]='\0';
  e->pron=u32_to_u8(pron,n,NULL,&n);
  if(e->pron==NULL)
    {
      free(e->word);
      return 0;
    }
  return 1;
}

static userdict load_entries(const char *path)
{
  FILE *fp=my_fopen(path,"rb");
  if(fp==NULL) return NULL;
  ustring32_t line=ustring32_alloc(64);
  ucs4_t c;
  unsigned int cs=0;
  entry e;
  entry *p;
  size_t n;
  userdict d=userdict_alloc(0,entry_free);
  while((c=ufgetc(fp,NULL,NULL))!=UEOF)
    {
      cs=classify_character(c);
      if(cs&cs_nl)
        {
          if(ustring32_empty(line)) continue;
          else
            {
              if(load_entry(line,&e))
                {
                  n=userdict_size(d);
                  if(n>0)
                    {
                      p=(entry*)lfind(&e,userdict_data(d),&n,sizeof(entry),cmp);
                      if(p!=NULL)
                        {
                          entry_free(p);
                          *p=e;
                        }
                      else userdict_push(d,&e);
                    }
                  else userdict_push(d,&e);
                }
              ustring32_clear(line);
            }
        }
      else ustring32_push(line,uc_tolower(c));
    }
  ustring32_free(line);
  fclose(fp);
  if(userdict_size(d)>0)
    {
      qsort(userdict_data(d),userdict_size(d),sizeof(entry),cmp);
      return d;
    }
  else
    {
      userdict_free(d);
      return NULL;
    }
}

static userdict dict=NULL;

int RHVoice_load_user_dict(const char *path)
{
  userdict d=load_entries(path);
  if(d==NULL) return 0;
  if(dict!=NULL) userdict_free(dict);
  dict=d;
  return 1;
}

void free_user_dict()
{
  if(dict!=NULL)
    {
      userdict_free(dict);
      dict=NULL;
    }
}

const uint8_t *user_dict_lookup(const uint8_t *word)
{
  if(dict==NULL) return NULL;
  entry key={(uint8_t*)word,NULL};
  const entry *p=(const entry*)bsearch(&key,userdict_data(dict),userdict_size(dict),sizeof(entry),cmp);
  return (p==NULL)?NULL:(p->pron);
}
