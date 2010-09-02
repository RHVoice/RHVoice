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

static int is_part_of_word(ucs4_t c,ucs4_t pc,ucs4_t nc)
{
  if(is_ru_letter(c))
    return TRUE;
  if(is_en_letter(c))
    return TRUE;
  if((c=='-')&&is_ru_letter(pc)&&is_ru_letter(nc))
    return TRUE;
  if((c=='\'')&&is_en_letter(pc)&&is_en_letter(nc))
    return true;
  return FALSE;
}

static int contains_russian_letters(const uint8_t *word)
{
  int result=FALSE;
  ucs4_t c;
  const uint8_t *str;
  for(str=u8_next(&c,word);str;str=u8_next(&c,str))
    {
      if(is_ru_letter(c))
        {
          result=TRUE;
          break;
        }
    }
  return result;
}

static int is_ru_cap(ucs4_t c)
{
  if((c>=1040)&&(c<=1071))
    return TRUE;
  if(c==1025)
    return TRUE;
  return FALSE;
}

static cst_val *ru_digits_to_words(const uint8_t *word)
{
  ucs4_t c;
  char d[]={'\0','\0'};
  const uint8_t *str;
  const uint8_t *end=word+u8_strlen(word);
  int i;
  cst_val *words=NULL;
  cst_val *lts_in=NULL;
  if((end-word>15)||*word=='0')
    {
      for(str=u8_prev(&c,end,word);str;str=u8_prev(&c,str,word))
        {
          words=cons_val(digit_to_word(c),words);
        }
    }
  else
    {
      for(i=1,str=u8_prev(&c,end,word);str;str=u8_prev(&c,str,word),i++)
        {
          switch(i)
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
          d[0]=c;
          lts_in=cons_val(string_val(d),lts_in);
        }
      words=ru_lts_apply(lts_in,&ru_number_lts);
      delete_val(lts_in);
    }
  return words;
}

cst_val *ru_tokentowords(const cst_item *t)
{
  ucs4_t pc,c,nc;
  const uint8_t *str=(uint8_t*)item_name(t);
  const uint8_t *nstr,*nnstr;
  uint8_t *word=u8_strdup(str);
  uint8_t *current=word;
  uint8_t *end=word+u8_strlen(word);
  cst_val *words=NULL;
  cst_val *nwords;
  const cst_val *v;
  cst_val *lts_in,*lts_out;
  char *tmp,*tmp1,*tmp2;
  int is_single_char;
  const char **e;
  pc='\0';
  nstr=u8_next(&c,str);
  while(nstr)
    {
      nnstr=u8_next(&nc,nstr);
      if(nnstr==NULL)
        nc='\0';
      is_single_char=!is_part_of_word(c,pc,nc)&&!is_digit(c);
      if(!is_single_char&&!is_digit(c))
        {
          current+=u8_uctomb(current,uc_tolower(c),end-current);
        }
      if((current>word)&&(is_single_char||is_digit(c)||(nc=='\0')))
        {
          *current='\0';
          e=NULL;
          if(feat_present(item_utt(t)->features,"user_dict"))
            e=ru_user_dict_lookup((const ru_user_dict*)val_userdata(feat_val(item_utt(t)->features,"user_dict")),word);
          if(e)
            {
              for(e++;*e!=NULL;e++)
                {
                  words=cons_val(string_val(*e),words);
                }
            }
          else
            if(get_param_int(item_utt(t)->features,"pseudo_english",1)&&!contains_russian_letters(word))
              words=cons_val(string_val((const char*)word),words);
            else
              if(u8_strcmp(word,(const char*)"й")==0)
                words=cons_val(string_val("краткое"),cons_val(string_val("и"),words));
              else
                if(u8_strcmp(word,(const char*)"ь")==0)
                  words=cons_val(string_val("знак"),cons_val(string_val("мягкий"),words));
                else
                  if(u8_strcmp(word,(const char*)"ъ")==0)
                    words=cons_val(string_val("знак"),cons_val(string_val("твёрдый"),words));
                  else
                    {
                      lts_in=cst_utf8_explode((const char*)word);
                      lts_out=ru_lts_apply(lts_in,&ru_downcase_lts);
                      delete_val(lts_in);
                      lts_in=lts_out;
                      lts_out=ru_lts_apply(lts_in,&ru_hyphen_lts);
                      delete_val(lts_in);
                      tmp=cst_implode(lts_out);
                      delete_val(lts_out);
                      tmp1=tmp;
                      while(tmp1)
                        {
                          tmp2=strchr(tmp1,'|');
                          if(tmp2)
                            *tmp2='\0';
                          words=cons_val(string_val(tmp1),words);
                          tmp1=tmp2?(tmp2+1):NULL;
                        }
                      cst_free(tmp);
                    }
          current=word;
        }
      if(is_digit(c))
        {
          nstr=str+u8_strspn(str,(const uint8_t*)"0123456789");
          nnstr=u8_next(&nc,nstr);
          if(nnstr==NULL)
            nc='\0';
          u8_cpy(word,str,(nstr-str));
          *(word+(nstr-str))='\0';
          for(nwords=ru_digits_to_words(word),v=nwords;v;v=val_cdr(v))
            {
              words=cons_val(val_car(v),words);
            }
          delete_val(nwords);
          u8_prev(&pc,nstr,str);
          c=nc;
          str=nstr;
          nstr=nnstr;
        }
      else
        {
          pc=c;
          c=nc;
          str=nstr;
          nstr=nnstr;
        }
    }
  free(word);
  words=val_reverse(words);
  return words;
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
