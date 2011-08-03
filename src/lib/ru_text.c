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
#include <unictype.h>
#include "lib.h"
#include "settings.h"
#include "ustring.h"
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

static cst_item *digits_to_words(cst_item *t,cst_relation *r,const uint8_t *digits,size_t n)
{
  cst_item *w=NULL;
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

static cst_item *character_to_words(cst_item *t,cst_relation *r,ucs4_t c,int spell)
{
  cst_item *w=NULL;
  const char *name=NULL;
  int uv=item_feat_int(t,"variant")-builtin_variant_count;
  if(uv<0) uv=0;
  uint8_t b[8];
  int n=u8_uctomb(b,c,sizeof(b));
  b[n]='\0';
  size_t p=0;
  if(uv>0) name=(const char*)user_dict_lookup(uv,b,&p);
  if(name==NULL) name=(const char*)user_dict_lookup(0,b,&p);
  if(name==NULL) name=character_name(c);
  if(name)
    w=add_words(t,r,name);
  else if(spell||uc_is_property(c,UC_PROPERTY_ALPHABETIC))
    {
      add_word(t,r,"символ");
      snprintf((char*)b,sizeof(b),"%u",c);
      w=digits_to_words(t,r,b,n);
    }
  return w;
}

cst_utterance *russian_textanalysis(cst_utterance *u)
{
  cst_relation *r=utt_relation_create(u,"Word");
  cst_item *t,*w;
  ustring8_t word=ustring8_alloc(0);
  if(word==NULL) return NULL;
  const uint8_t *text,*s1,*s2,*pron;
  size_t pos;
  size_t len;
  int say_as;
  int spell;
  ucs4_t c,lc,pc;
  unsigned int flags;
  int uv;
  int punct_mode;
  const uint32_t *punct_list;
  for(t=relation_head(utt_relation(u,"Token"));t;t=item_next(t))
    {
      say_as=item_feat_present(t,"say_as")?item_feat_int(t,"say_as"):0;
      spell=((say_as=='s')||(say_as=='c'));
      text=(const uint8_t*)item_feat_string(t,"text");
      if(text[0]=='\0') continue;
      uv=item_feat_int(t,"variant")-builtin_variant_count;
      if(uv<0) uv=0;
      punct_mode=item_feat_int(t,"punct_mode");
      if(punct_mode==RHVoice_punctuation_some)
        punct_list=(const uint32_t*)val_userdata(item_feat(t,"punct_list"));
      else
        punct_list=NULL;
      pc='\0';
      s1=text;
      s2=u8_next(&c,s1);
      while(s2)
        {
          pos=s1-text;
          lc=uc_tolower(c);
          flags=classify_character(c);
          if((!spell)&&(stress_marker!='\0')&&(pc==stress_marker)&&(flags&cs_ru)&&(flags&cs_lv))
            ustring8_push(word,'+');
          if(spell)
            {
              w=character_to_words(t,r,c,1);
              if(w!=NULL)
                item_set_string(w,"my_gpos","content");
            }
          else if((stress_marker!='\0')&&(c==stress_marker));
          else if(uc_is_general_category(c,UC_PUNCTUATION)||uc_is_general_category(c,UC_SYMBOL))
            {
              if(!ustring8_empty(word))
                {
                  w=add_word(t,r,(const char*)ustring8_str(word));
                  ustring8_clear(word);
                }
              if((punct_mode==RHVoice_punctuation_all)||((punct_mode==RHVoice_punctuation_some)&&u32_strchr(punct_list,c)))
                w=character_to_words(t,r,c,0);
            }
          else if(((uv>0)&&(pron=user_dict_lookup(uv,text,&pos)))||
                  (pron=user_dict_lookup(0,text,&pos)))
            {
              s2=text+pos;
              len=u8_strlen(pron);
              if(pron[len+1]!='\0')
                {
                  if(!ustring8_empty(word))
                    {
                      w=add_word(t,r,(const char*)ustring8_str(word));
                      ustring8_clear(word);
                    }
                  w=add_words(t,r,(const char*)pron);
                }
              else
                ustring8_append(word,pron,len);
            }
          else if(flags&cs_l)
            {
              ustring8_push(word,lc);
            }
          else if(flags&cs_d)
            {
              if(!ustring8_empty(word))
                {
                  w=add_word(t,r,(const char*)ustring8_str(word));
                  ustring8_clear(word);
                }
              s2=s1+u8_strspn(s1,(const uint8_t*)"0123456789");
              if(((s1-text)==1)&&(text[0]=='-')) add_word(t,r,"минус");
          w=digits_to_words(t,r,s1,s2-s1);
            }
          else
            {
              if(!ustring8_empty(word))
                {
                  w=add_word(t,r,(const char*)ustring8_str(word));
                  ustring8_clear(word);
                }
              w=character_to_words(t,r,c,0);
            }
          pc=c;
          s1=s2;
          s2=u8_next(&c,s1);
        }
      if(!ustring8_empty(word))
        {
          w=add_word(t,r,(const char*)ustring8_str(word));
          ustring8_clear(word);
        }
    }
  ustring8_free(word);
  return u;
}
