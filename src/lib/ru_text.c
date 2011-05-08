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
#include <unistdio.h>
#include "lib.h"
#include "settings.h"
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

static cst_item *digits_to_words(cst_item *t,cst_relation *r,const uint32_t *digits)
{
  cst_item *w=NULL;
  size_t n=u32_strlen(digits);
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
  const char *name=NULL;
  uint32_t ls[2];
  ls[0]=uc_tolower(c);
  ls[1]='\0';
  ustring8_t name0=NULL;
  int uv=item_feat_int(t,"variant")-builtin_variant_count;
  if(uv<0) uv=0;
  if((uv>0)&&
     ((name=(const char*)user_variant_lookup(uv,ls))||
      (name0=user_variant_apply(uv,ls))))
    {
      if(name!=NULL) w=add_words(t,r,name);
      else
        {
          w=add_words(t,r,(const char*)ustring8_str(name0));
          ustring8_free(name0);
        }
    }
  else if((name=(const char*)user_dict_lookup32(global_user_dict,ls))||
          (name=character_name(c)))
    {
      w=add_words(t,r,name);
    }
  else
    {
      add_word(t,r,"символ");
      uint32_t b[10];
      u32_sprintf(b,"%u",c);
      w=digits_to_words(t,r,b);
    }
  return w;
}

static const char *unsep_words[]={"из-за","из-под",NULL};

static cst_item *ru_letters_to_words(cst_item *t,cst_relation *r,const uint32_t *s32)
{
  cst_item *w=NULL;
  const uint8_t *pr=user_dict_lookup32(global_user_dict,s32);
  if(pr!=NULL)
    return add_words(t,r,(const char*)pr);
  size_t n=0;
  uint8_t *s8=u32_to_u8(s32,u32_strlen(s32)+1,NULL,&n);
  if(s8==NULL) return NULL;
  if(cst_member_string((const char*)s8,unsep_words))
    {
      w=add_word(t,r,(const char*)s8);
      free(s8);
      return w;
    }
  const uint8_t *enc=NULL;
  uint8_t *p=u8_strrchr(s8,'-');
  if((p!=NULL)&&cst_member_string((const char*)(p+1),ru_h_enc_list))
    {
      *p='\0';
      enc=p+1;
    }
  p=NULL;
  uint8_t *s=u8_strtok(s8,(const uint8_t*)"-",&p);
  while(s)
    {
      if((pr=user_dict_lookup8(global_user_dict,s)))
        w=add_words(t,r,(const char*)pr);
      else
        w=add_word(t,r,(const char*)s);
      s=u8_strtok(NULL,(const uint8_t*)"-",&p);
    }
  if(enc!=NULL)
    {
      w=add_word(t,r,(const char*)enc);
      item_set_string(w,"my_gpos","enc");
    }
  free(s8);
  return w;
}

static cst_item *user_variant_letters_to_words(cst_item *t,cst_relation *r,const uint32_t *s32,int uv)
{
  cst_item *w=NULL;
  const uint8_t *pr1=NULL;
  if(!((s32[1]=='\0')&&user_variant_is_member(uv,s32[0],'W'))&&(pr1=user_variant_lookup(uv,s32)))
    {
      w=add_words(t,r,(const char*)pr1);
      return w;
    }
  ustring8_t pr2=user_variant_apply(uv,s32);
  if(pr2!=NULL)
    {
      w=add_words(t,r,(const char*)ustring8_str(pr2));
      ustring8_free(pr2);
      return w;
    }
  return NULL;
}

cst_utterance *russian_textanalysis(cst_utterance *u)
{
  cst_relation *r=utt_relation_create(u,"Word");
  cst_item *t,*w;
  size_t n,i;
  ustring32_t text=ustring32_alloc(0);
  if(text==NULL) return NULL;
  ustring32_t word=ustring32_alloc(0);
  if(word==NULL)
    {
      ustring32_free(text);
      return NULL;
    }
  int say_as;
  int spell;
  ucs4_t pc,c,nc,lc,plc,nlc;
  unsigned int pflags,flags,nflags;
  int uv;
  for(t=relation_head(utt_relation(u,"Token"));t;t=item_next(t))
    {
      say_as=item_feat_present(t,"say_as")?item_feat_int(t,"say_as"):0;
      spell=((say_as=='s')||(say_as=='c'));
      ustring32_assign8(text,(const uint8_t*)item_feat_string(t,spell?"text":"name"));
      if(ustring32_empty(text)) continue;
      uv=item_feat_int(t,"variant")-builtin_variant_count;
      if(uv<0) uv=0;
      n=ustring32_length(text);
      pc='\0';
      plc='\0';
      pflags=cs_sp;
      c=ustring32_at(text,0);
      lc=uc_tolower(c);
      flags=classify_character(c);
      for(i=0;i<n;i++)
        {
          nc=(i==(n-1))?'\0':ustring32_at(text,i+1);
          nlc=uc_tolower(nc);
          nflags=classify_character(nc);
          if(spell)
            {
              w=character_to_words(t,r,c);
              if(w!=NULL)
                item_set_string(w,"my_gpos","content");
            }
          else if(uv&&user_variant_is_alpha(uv,lc))
            {
              ustring32_push(word,lc);
              if(!user_variant_is_alpha(uv,nlc))
                {
                  w=user_variant_letters_to_words(t,r,ustring32_str(word),uv);
                  ustring32_clear(word);
                }
            }
          else if(flags&cs_l)
            {
              ustring32_push(word,lc);
              if((uv&&user_variant_is_alpha(uv,nlc))||(!((nflags&cs_l)||((nc=='\'')&&(flags&cs_en))||((nc=='-')&&(flags&cs_ru)))))
                {
                  w=ru_letters_to_words(t,r,ustring32_str(word));
                  ustring32_clear(word);
                }
            }
          else if(flags&cs_d)
            {
              ustring32_push(word,c);
              if(!(nflags&cs_d))
                {
                  w=digits_to_words(t,r,ustring32_str(word));
                  ustring32_clear(word);
                }
            }
          else if(flags&cs_s)
            {
              w=character_to_words(t,r,c);
            }
          else if(c=='-')
            {
              if((!ustring32_empty(word))&&(pflags&cs_l)&&(pflags&cs_ru))
                {
                  if((nflags&cs_l)&&(nflags&cs_ru)&&(!(uv&&user_variant_is_alpha(uv,nlc))))
                    ustring32_push(word,c);
                  else
                    {
                      w=ru_letters_to_words(t,r,ustring32_str(word));
                      ustring32_clear(word);
                    }
                }
              else if((nflags&cs_d)&&(i==0))
                {
                  w=add_word(t,r,"минус");
                }
            }
          else if(c=='\'')
            {
              if((!ustring32_empty(word))&&(pflags&cs_l)&&(pflags&cs_en))
                {
                  if((nflags&cs_l)&&(nflags&cs_en)&&(!(uv&&user_variant_is_alpha(uv,nlc))))
                    ustring32_push(word,c);
                  else
                    {
                      w=ru_letters_to_words(t,r,ustring32_str(word));
                      ustring32_clear(word);
                    }
                }
            }
          else if(flags&cs_p);
          else w=character_to_words(t,r,c);
          pc=c;
          pflags=flags;
          plc=lc;
          c=nc;
          flags=nflags;
          lc=nlc;
        }
    }
  ustring32_free(word);
  ustring32_free(text);
  return u;
}
