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

#include "lib.h"

extern const cst_cart ru_vowel_reduction_cart;
extern const cst_cart ru_vpair_cart;
static const char *unstressed_vowels[]={"a","o","u","e","i","y",NULL};

cst_utterance *russian_postlex_function(cst_utterance *u)
{
  const cst_item *word,*seg;
  const char *answer,*name,*pair;
  for(word=relation_head(utt_relation(u,"Transcription"));word;word=item_next(word))
    {
      if(item_feat_present(word,"no_pl")||item_feat_present(word,"no_vr"))
        continue;
      for(seg=item_daughter(word);seg;seg=item_next(seg))
        {
          name=item_feat_string(seg,"name");
          if(cst_member_string(name,unstressed_vowels))
            {
              answer=val_string(cart_interpret(item_as(seg,"Segment"),&ru_vowel_reduction_cart));
              if(!cst_streq(answer,"N"))
                item_set_string(seg,"name",answer);
            }
          else
            {
              if(cst_streq(name,"ii")&&
                 cst_streq(ffeature_string(seg,"R:Segment.p.ph_csoft"),"-")&&
                 !(cst_streq(item_feat_string(word,"name"),"и")&&
                   cst_streq(ffeature_string(word,"gpos"),"content")))
                {
                  item_set_string(seg,"name","yy");
                }
            }
        }
    }
  for(word=relation_tail(utt_relation(u,"Transcription"));word;word=item_prev(word))
    {
      if(item_feat_present(word,"no_pl"))
        continue;
      for(seg=item_last_daughter(word);seg;seg=item_prev(seg))
        {
          name=item_feat_string(seg,"name");
          pair=russian_vpair(name);
          if(pair!=NULL)
            {
              answer=val_string(cart_interpret(item_as(seg,"Segment"),&ru_vpair_cart));
              if(cst_streq(answer,"Y"))
                item_set_string(seg,"name",pair);
            }
        }
    }
  return u;
}
