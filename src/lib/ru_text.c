/* Copyright (C) 2009, 2010  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include "russian.h"
#include <string.h>
#include <unistr.h>
#include <unicase.h>
#include "ru_number_lts.h"
#include "ru_downcase_lts.h"
#include "ru_hyphen_lts.h"

DEF_STATIC_CONST_VAL_STRING(zero,"ноль");
DEF_STATIC_CONST_VAL_STRING(one,"один");
DEF_STATIC_CONST_VAL_STRING(two,"два");
DEF_STATIC_CONST_VAL_STRING(three,"три");
DEF_STATIC_CONST_VAL_STRING(four,"четыре");
DEF_STATIC_CONST_VAL_STRING(five,"пять");
DEF_STATIC_CONST_VAL_STRING(six,"шесть");
DEF_STATIC_CONST_VAL_STRING(seven,"семь");
DEF_STATIC_CONST_VAL_STRING(eight,"восемь");
DEF_STATIC_CONST_VAL_STRING(nine,"девять");

static const cst_val * digits[]={
  &zero,
  &one,
  &two,
  &three,
  &four,
  &five,
  &six,
  &seven,
  &eight,
  &nine};

DEF_STATIC_CONST_VAL_STRING(thousand_symbol,"th");
DEF_STATIC_CONST_VAL_STRING(million_symbol,"m");
DEF_STATIC_CONST_VAL_STRING(billion_symbol,"b");
DEF_STATIC_CONST_VAL_STRING(trillion_symbol,"tr");

static const cst_val *digit_to_word(ucs4_t c)
{
  return digits[c-48];
}

static int is_ru_letter(ucs4_t c)
{
  if((c>=1072)&&(c<=1103)) //from Russian a to Russian ya
    return TRUE;
  if((c>=1040)&&(c<=1071)) //from capital Russian a to capital Russian ya
    return TRUE;
  if(c==1105) //Russian yo
    return TRUE;
  if(c==1025) //capital Russian yo
    return TRUE;
  return FALSE;
}

static int is_en_letter(ucs4_t c)
{
  if((c>='a')&&(c<='z'))
    return TRUE;
  if((c>='A')&&(c<='Z'))
    return TRUE;
  return FALSE;
}

static int is_digit(ucs4_t c)
{
  return ((c>='0')&&(c<='9'));
}

static int is_known_char(ucs4_t c)
{
  if(is_ru_letter(c))
    return TRUE;
  if(is_en_letter(c))
    return TRUE;
  if(is_digit(c))
    return TRUE;
  if(c=='-')
    return TRUE;
  if(c=='\'')
    return true;
  if(c=='+')
    return TRUE;
  return FALSE; //ignore everything else (at least for now)
}

static int is_ru_cap(ucs4_t c)
{
  if((c>=1040)&&(c<=1071))
    return TRUE;
  if(c==1025)
    return TRUE;
  return FALSE;
}

static cst_val *ru_tokentowords2(const char *name)
{
  int l,i;
  char d[]={'\0','\0'};
  cst_val *words=NULL;
  cst_val *lts_output=NULL;
  cst_val *lts_input=NULL;
  const cst_val *v=NULL;
  uint8_t *buff=(uint8_t*)strdup(name);
  const uint8_t *ptr1=(uint8_t*)name;
  uint8_t *ptr2=buff;
  const uint8_t *ptr3=NULL;
  ucs4_t c;
  ptr1=u8_next(&c,ptr1);
  while(ptr1)
    {
      if(!is_known_char(c))
        {
          if(ptr2!=buff)
            {
              *ptr2='\0';
              words=cons_val(string_val((char*)buff),words);
              ptr2=buff;
            }
          ptr1=u8_next(&c,ptr1);
          continue;
        }
      if(is_digit(c))
        {
          if(ptr2!=buff)
            {
              *ptr2='\0';
              words=cons_val(string_val((char*)buff),words);
              ptr2=buff;
            }
          l=u8_strspn(ptr1,(const uint8_t*)"0123456789");
          if((l>14)||(c==48))
            {
              ptr3=ptr1-1;
              ptr1+=l;
              do
                {
                  ptr3=u8_next(&c,ptr3);
                  words=cons_val(digit_to_word(c),words);
                }
              while(ptr3!=ptr1);
            }
          else
            {
              ptr3=ptr1+l;
              i=1;
              for(;ptr3>=ptr1;ptr3--,i++)
                {
                  switch(i)
                    {
                    case 4:
                      lts_input=cons_val(&thousand_symbol,lts_input);
                      break;
                    case 7:
                      lts_input=cons_val(&million_symbol,lts_input);
                      break;
                    case 10:
                      lts_input=cons_val(&billion_symbol,lts_input);
                      break;
                    case 13:
                      lts_input=cons_val(&trillion_symbol,lts_input);
                      break;
                    default:
                      break;
                    }
                  d[0]=(char)*(ptr3-1);
                  lts_input=cons_val(string_val(d),lts_input);
                }
              lts_output=ru_lts_apply(lts_input,&ru_number_lts);
              for(v=lts_output;v;v=val_cdr(v))
                {
                  words=cons_val(val_car(v),words);
                }
              if(lts_input)
                {
                  delete_val(lts_input);
                  lts_input=NULL;
                }
              if(lts_output)
                {
                  delete_val(lts_output);
                  lts_output=NULL;
                }
              ptr1+=l;
            }
          ptr1=u8_next(&c,ptr1);
          continue;
        }
      c=uc_tolower(c);
      ptr2+=u8_uctomb(ptr2,c,2);
      ptr1=u8_next(&c,ptr1);
    }
  if(ptr2!=buff)
    {
      *ptr2='\0';
      words=cons_val(string_val((char*)buff),words);
    }
  if(buff)
    free(buff);
  words=val_reverse(words);
  return words;
}

cst_val *ru_tokentowords(const cst_item *t)
{
  cst_val *words1=ru_tokentowords2(item_name(t));
  char *word;
  char *p1,*p2;
  cst_val *words2=NULL;
  const cst_val *v;
  cst_val *lts_in,*lts_out;
  ucs4_t c;
  const uint8_t *ustr;
  int has_en_letters,has_ru_letters;
  for(v=words1;v;v=val_cdr(v))
    {
      ustr=(uint8_t*)val_string(val_car(v));
      has_en_letters=FALSE;
      has_ru_letters=FALSE;
      for(ustr=u8_next(&c,ustr);ustr;ustr=u8_next(&c,ustr))
        {
          if(is_en_letter(c))
            has_en_letters=TRUE;
          if(is_ru_letter(c))
            has_ru_letters=TRUE;
        }
      if(has_en_letters&&!has_ru_letters)
        {
          words2=cons_val(val_car(v),words2);
          continue;
        }
      if(cst_streq(val_string(val_car(v)),"й"))
        {
          words2=cons_val(string_val("краткое"),cons_val(string_val("и"),words2));
          continue;
        }
      if(cst_streq(val_string(val_car(v)),"ь"))
        {
          words2=cons_val(string_val("знак"),cons_val(string_val("мягкий"),words2));
          continue;
        }
      if(cst_streq(val_string(val_car(v)),"ъ"))
        {
          words2=cons_val(string_val("знак"),cons_val(string_val("твёрдый"),words2));
          continue;
        }
      lts_in=cst_utf8_explode(val_string(val_car(v)));
      lts_out=ru_lts_apply(lts_in,&ru_downcase_lts);
      delete_val(lts_in);
      if(lts_out==NULL)
        continue;
      lts_in=lts_out;
      lts_out=ru_lts_apply(lts_in,&ru_hyphen_lts);
      delete_val(lts_in);
      if(lts_out==NULL)
        continue;
      word=cst_implode(lts_out);
      delete_val(lts_out);
      p1=word;
      while(p1)
        {
          p2=strchr(p1,'|');
          if(p2)
            *p2='\0';
          words2=cons_val(string_val(p1),words2);
          p1=p2?(p2+1):NULL;
        }
      cst_free(word);
    }
  delete_val(words1);
  words2=val_reverse(words2);
  return words2;
}

int ru_utt_break(cst_tokenstream *ts, const char *token, cst_relation *tokens)
{
  int result=FALSE;
  const char *postpunct = item_feat_string(relation_tail(tokens), "punc");
  const char *ltoken = item_name(relation_tail(tokens));
  ucs4_t pc='\0';
  ucs4_t nc='\0';
  int ltlen=u8_strlen((const uint8_t*)ltoken);
  u8_next(&nc,(const uint8_t*)token);
  u8_prev(&pc,((const uint8_t*)ltoken)+ltlen,(const uint8_t*)ltoken);
  char *full_token=(char*)malloc(sizeof(char)*(strlen(ts->prepunctuation)+strlen(token)+strlen(ts->postpunctuation)+1));
  strcpy(full_token,ts->prepunctuation);
  strcat(full_token,token);
  strcat(full_token,ts->postpunctuation);
  if(strchr(postpunct,';'))
    result=TRUE;
  else if(strchr(postpunct,':'))
    {
      if(strchr(ts->whitespace,'\n')&&(full_token[0]=='-'))
        // probably dialogue
        result=TRUE;
      else if(full_token[0]=='\"')
        // direct speech
        result=TRUE;
      else result=FALSE;
    }
  else if(strchr(postpunct,'?')||strchr(postpunct,'!'))
    {
      if(u8_endswith((const uint8_t*)postpunct,(const uint8_t*)"-"))
        // probably followed by the authors words
        result=FALSE;
      else if(u8_endswith((const uint8_t*)postpunct,(const uint8_t*)"\"")&&(full_token[0]=='-'))
        // same as above
        result=FALSE;
      result=TRUE;
    }
  else if(strchr(postpunct,'.')&&is_ru_cap(nc))
    {
      if(postpunct[1]=='\0')
        {
          if((ltlen==1)&&is_ru_cap(pc))
            result=FALSE;
          else result=TRUE;
        }
      else result=TRUE;
    }
  free(full_token);
  return result;
}
