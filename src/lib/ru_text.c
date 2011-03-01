/* Copyright (C) 2009, 2010, 2011  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include <stdio.h>
#include <string.h>
#include <unistr.h>
#include <unicase.h>
#include "lib.h"
#include "ru_number_lts.h"

DEF_STATIC_CONST_VAL_STRING(thousand_symbol,"th");
DEF_STATIC_CONST_VAL_STRING(million_symbol,"m");
DEF_STATIC_CONST_VAL_STRING(billion_symbol,"b");
DEF_STATIC_CONST_VAL_STRING(trillion_symbol,"tr");

static cst_item *add_word(cst_item *t,cst_relation *r,const char *name)
{
  cst_item *w=item_add_daughter(t,NULL);
  item_set_string(w,"name",name);
  relation_append(r,w);
  return w;
}

static cst_item *add_words(cst_item *t,cst_relation *r,const char *strlist)
{
  cst_item *w=NULL;
  const char *str=strlist;
  while((str[0]!='\0'))
    {
      w=add_word(t,r,str);
      str+=(strlen(str)+1);
    }
  return w;
}

static cst_item *digits_to_words(cst_item *t,cst_relation *r,const uint8_t *digits)
{
  cst_item *w=NULL;
  size_t n=u8_strlen(digits);
  size_t i;
  if((n>15)||(digits[0]=='0'))
    {
      for(i=0;i<n;i++)
        {
          w=add_word(t,r,character_name(digits[i]));
        }
    }
  else
    {
      char d[]={'\0','\0'};
      cst_val *words=NULL;
      cst_val *lts_in=NULL;
      const cst_val *v=NULL;
      i=n;
      do
        {
          i--;
          switch(n-i)
            {
            case 4:
              lts_in=cons_val(&thousand_symbol,lts_in);
              break;
            case 7:
              lts_in=cons_val(&million_symbol,lts_in);
              break;
            case 10:
              lts_in=cons_val(&billion_symbol,lts_in);
              break;
            case 13:
              lts_in=cons_val(&trillion_symbol,lts_in);
              break;
            default:
              break;
            }
          d[0]=digits[i];
          lts_in=cons_val(string_val(d),lts_in);
        }
      while(i);
      words=ru_lts_apply(lts_in,&ru_number_lts);
      delete_val(lts_in);
      for(v=words;v;v=val_cdr(v))
        {
          w=add_word(t,r,val_string(val_car(v)));
        }
      delete_val(words);
    }
  return w;
}

static cst_item *character_to_words(cst_item *t,cst_relation *r,ucs4_t c)
{
  cst_item *w=NULL;
  const char *name=character_name(c);
  if(name!=NULL)
    {
      w=add_words(t,r,name);
    }
  else
    {
      add_word(t,r,"символ");
      char b[10];
      sprintf(b,"%u",c);
      w=digits_to_words(t,r,(const uint8_t*)b);
    }
  return w;
}

cst_utterance *russian_textanalysis(cst_utterance *u)
{
  cst_relation *r=utt_relation_create(u,"Word");
  cst_item *t,*w;
  const char *name,*pname;
  size_t n,i;
  ustring32_t text=ustring32_alloc(0);
  if(text==NULL) return NULL;
  ustring8_t word=ustring8_alloc(0);
  if(word==NULL)
    {
      ustring32_free(text);
      return NULL;
    }
  int say_as;
  int spell;
  ucs4_t pc,c,nc;
  unsigned int pflags,flags,nflags;
  int in_ru_compound_word;
  for(t=relation_head(utt_relation(u,"Token"));t;t=item_next(t))
    {
      ustring8_clear(word);
      say_as=item_feat_present(t,"say_as")?item_feat_int(t,"say_as"):0;
      spell=((say_as=='s')||(say_as=='c'));
      ustring32_assign8(text,(const uint8_t*)item_feat_string(t,spell?"text":"name"));
      if(ustring32_empty(text)) continue;
      n=ustring32_length(text);
      pc='\0';
      pflags=cs_sp;
      c=ustring32_at(text,0);
      flags=classify_character(c);
      in_ru_compound_word=0;
      for(i=0;i<n;i++)
        {
          nc=(i==(n-1))?'\0':ustring32_at(text,i+1);
          nflags=classify_character(nc);
          if(spell)
            {
              w=character_to_words(t,r,c);
              if(flags&cs_lw)
                item_set_string(w,"my_gpos","content");
            }
          else if((flags&cs_l)||((c=='\'')&&(pflags&(cs_l|cs_en))))
            {
              ustring8_push(word,uc_tolower(c));
              if(!((nflags&cs_l)||((nc=='\'')&&(flags&(cs_l|cs_en)))))
                {
                  if(in_ru_compound_word)
                    {
                      name=(const char*)ustring8_str(word);
                      pname=item_feat_string(w,"name");
                      if(cst_member_string(name,ru_h_enc_list))
                        {
                          w=add_word(t,r,name);
                          item_set_string(w,"my_gpos","enc");
                        }
                      else if(cst_streq(pname,"из"))
                        {
                          if(cst_streq(name,"за"))
                            item_set_string(w,"name","из-за");
                          else if(cst_streq(name,"под"))
                            item_set_string(w,"name","из-под");
                          else w=add_word(t,r,name);
                        }
                      else w=add_word(t,r,name);
                    }
                  else w=add_word(t,r,(const char*)ustring8_str(word));
                  ustring8_clear(word);
                  in_ru_compound_word=0;
                }
            }
          else if(flags&cs_d)
            {
              ustring8_push(word,c);
              if(!(nflags&cs_d))
                {
                  w=digits_to_words(t,r,ustring8_str(word));
                  ustring8_clear(word);
                }
            }
          else if(flags&cs_s)
            {
              w=character_to_words(t,r,c);
            }
          else if((c=='-')&&(pflags&(cs_l|cs_ru))&&(nflags&(cs_l|cs_ru)))
            {
              in_ru_compound_word=1;
            }
          pc=c;
          pflags=flags;
          c=nc;
          flags=nflags;
        }
    }
  ustring8_free(word);
  ustring32_free(text);
  return u;
}
