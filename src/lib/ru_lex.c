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

#include "lib.h"
#include <unistr.h>
#include "ru_dict.h"
#include "ru_lts.h"
#include "ru_consonants_lts.h"
#include "en_consonants_lts.h"
#include "ru_en_lts.h"
#include <string.h>
#include <stdlib.h>

extern const cst_cart ru_stress_cart;


static int compare_entries(const void *key,const void *element)
{
  return strcmp((const char*)key,((const ru_dict_entry*)element)->word);
}

static int is_silence(const char *p)
{
  if (cst_streq(p,"pau"))
    return TRUE;
  else
    return FALSE;
}

static int is_vowel(const char *p)
{
  if (strchr("aeiouy",p[0]) == NULL)
    return FALSE;
  else
    return TRUE;
}

static int is_stressed_vowel(const char *p)
{
  return is_vowel(p)&&(p[0]==p[1]);
}

static int is_sonorant(const char *p)
{
  if (strchr("lmnr",p[0]) == NULL)
    return FALSE;
  else
    return TRUE;
}

static int is_j(const char *p)
{
  if (!p) return FALSE;
  return (p[0]=='j');
}

static int has_vowel_in_list(const cst_val *v)
{
  const cst_val *t;

  for (t=v; t; t=val_cdr(t))
    if (is_vowel(val_string(val_car(t))))
      return TRUE;
  return FALSE;
}

static int has_vowel_in_syl(const cst_item *i)
{
  const cst_item *n;

  for (n=i; n; n=item_prev(n))
    if (is_vowel(item_feat_string(n,"name")))
      return TRUE;
  return FALSE;
}

static int ru_syl_boundary(const cst_item *i,const cst_val *v)
{
  const char *p;
  const char *n;
  const char *nn;
  if (v == NULL)
    return TRUE;
  n=val_string(val_car(v));
  if (is_silence(n))
    return TRUE;
  if (!has_vowel_in_list(v))
    return FALSE;
  if (!has_vowel_in_syl(i))
    return FALSE;
  if (is_vowel(n))
    return TRUE;
  if (val_cdr(v) == NULL)
    return FALSE;
  p = item_feat_string(i,"name");
  nn = val_string(val_car(val_cdr(v)));
  if(is_vowel(p))
    {
      if(is_vowel(nn))
        return TRUE;
      if(is_sonorant(n)&&!is_sonorant(nn))
        return FALSE;
      if(is_j(n))
        return FALSE;
      return TRUE;
    }
  if(is_sonorant(p))
    return TRUE;
  if(is_j(p))
    return TRUE;
  return FALSE;
}

static int vowel_seg_between(const cst_item *f,const cst_item *l)
{
  const cst_item *s;
  for(s=f;s;s=item_next(s))
    {
      if(is_vowel(item_feat_string(s,"name")))
        return TRUE;
      if(item_equal(s,l))
        break;
    }
  return FALSE;
}

static cst_val* word_to_phones(const cst_item *word)
{
  cst_val*phones=NULL;
  const char *name=item_feat_string(word, "name");
  ustring32_t letters=ustring32_alloc(0);
  if(letters==NULL) return NULL;
  ustring32_assign8(letters,(const uint8_t*)name);
  if(ustring32_empty(letters))
    {
      ustring32_free(letters);
      return NULL;
    }
  unsigned int flags=classify_characters(ustring32_str(letters),ustring32_length(letters));
  cst_lexicon *cmu_lex=val_lexicon(feat_val(item_utt(word)->features,"cmu_lex"));
  int variant=item_feat_int(item_parent(item_as(word,"Token")),"variant");
  if((flags&cs_lc)&&cst_streq(ffeature_string(word,"gpos"),"content"))
    {
      if(variant==RHVoice_variant_pseudo_english)
        phones=ustring32_lts_apply(letters,&en_consonants_lts);
      else phones=ustring32_lts_apply(letters,&ru_consonants_lts);
      item_set_int(word,"no_vr",1);
    }
  else if((variant==RHVoice_variant_pseudo_english)&&(flags&cs_en))
    {
      cst_val *en_phones=lex_lookup(cmu_lex,name,(cst_streq(name,"a")?"n":NULL));
      if(en_phones)
        {
          phones=ru_lts_apply(en_phones,&ru_en_lts);
          delete_val(en_phones);
        }
      item_set_int(word,"no_pl",1);
    }
  else
    {
      const ru_dict_entry *e=bsearch(name,ru_dict,ru_dict_size,sizeof(ru_dict_entry),compare_entries);
      if(e!=NULL)
        {
          if(e->stress > 0)
            ustring32_set(letters,e->stress-1,1105);
          else
            item_set_int(word,"stressed_syl_num",e->stress);
        }
      phones=ustring32_lts_apply(letters,&ru_lts);
    }
  ustring32_free(letters);
  return phones;
}

static void assign_stress(cst_item *word)
{
  int numsyls=ffeature_int(word,"word_numsyls");
  const char *gpos=ffeature_string(word,"gpos");
  const cst_item *word_in_phrase=item_as(word,"Phrase");
  const cst_item *syls=item_as(word,"SylStructure");
  const cst_item *syl=item_daughter(syls);
  const cst_item *transcription=item_as(word,"Transcription");
  int stressed=FALSE;
  int n=item_feat_present(word,"stressed_syl_num")?item_feat_int(word,"stressed_syl_num"):0;
  const char *name=item_name(word);
  const char *pname=ffeature_string(word,"R:Phrase.p.name");
  const char *nname=ffeature_string(word,"R:Phrase.n.name");
  if((cst_streq(name,"не")||cst_streq(name,"ни"))&&
     (cst_streq(nname,"был")||cst_streq(nname,"были")||cst_streq(nname,"было")))
    {
      item_set_string(syl,"stress","1");
      return;
    }
  if((cst_streq(name,"был")||cst_streq(name,"были")||cst_streq(name,"было"))&&
     (cst_streq(pname,"не")||cst_streq(pname,"ни")))
    return;
  if(cst_streq(gpos,"enc")&&item_prev(word_in_phrase))
    return;
  if(cst_streq(gpos,"proc")&&item_next(word_in_phrase))
    return;
  if(!vowel_seg_between(item_daughter(transcription),item_last_daughter(transcription)))
    return;
  if(numsyls==1)
    {
      item_set_string(syl,"stress","1");
      return;
    }
  for(;syl;syl=item_next(syl))
    {
      if(is_stressed_vowel(item_feat_string(item_daughter(item_as(syl,"SylVowel")),"name")))
        {
          item_set_string(syl,"stress","1");
          stressed=TRUE;
        }
    }
  if(n==0)
    {
      if(stressed)
        return;
      n=val_int(cart_interpret(word,&ru_stress_cart));
      if((numsyls+n) < 0)
        {
          if(numsyls <= 4)
            n=-2;
          else
            if(numsyls <= 6)
              n=-3;
            else
              n=-4;
        }
    }
  item_set_string(item_nth_daughter(syls,(numsyls+n)),"stress","1");
}

cst_utterance *russian_lexical_insertion(cst_utterance *u)
{
  cst_item *word;
  cst_relation *sylstructure,*seg,*syl,*sylvowel,*transcription;
  const cst_val *p;
  const char *phone_name;
  cst_val *phones;
  cst_item *ssword, *sssyl, *segitem, *sylitem, *seg_in_syl, *svsyl, *vowel_in_syl, *tword, *seg_in_word;
  cst_item *i,*tmp;
  syl = utt_relation_create(u,"Syllable");
  sylstructure = utt_relation_create(u,"SylStructure");
  seg = utt_relation_create(u,"Segment");
  sylvowel = utt_relation_create(u,"SylVowel");
  transcription = utt_relation_create(u,"Transcription");
  for (word=relation_head(utt_relation(u,"Word"));word;word=item_next(word))
    {
      phones=word_to_phones(word);
      if(!phones)
        continue;
      ssword = relation_append(sylstructure,word);
      tword = relation_append(transcription,word);
      for (sssyl=NULL,sylitem=NULL,p=phones; p; p=val_cdr(p))
        {
          if (sylitem == NULL)
            {
              sylitem = relation_append(syl,NULL);
              sssyl = item_add_daughter(ssword,sylitem);
            }
          segitem = relation_append(seg,NULL);
          phone_name = val_string(val_car(p));
          item_set_string(segitem,"name",phone_name);
          seg_in_syl = item_add_daughter(sssyl,segitem);
          seg_in_word = item_add_daughter(tword,segitem);
          if(is_vowel(phone_name))
            {
              svsyl=relation_append(sylvowel,sylitem);
              vowel_in_syl=item_add_daughter(svsyl,segitem);
            }
          if (ru_syl_boundary(seg_in_syl,val_cdr(p)))
            {
              sylitem = NULL;
              if (sssyl)
                item_set_string(sssyl,"stress","0");
            }
        }
      assign_stress(word);
      delete_val(phones);
    }
  i=relation_head(utt_relation(u,"Word"));
  while(i)
    {
      tmp=item_next(i);
      if(item_as(i,"Transcription")==NULL)
        delete_item(i);
      i=tmp;
    }
  i=relation_head(utt_relation(u,"Phrase"));
  while(i)
    {
      tmp=item_next(i);
      if(item_daughter(i)==NULL)
        delete_item(i);
      i=tmp;
    }
  return u;
}
