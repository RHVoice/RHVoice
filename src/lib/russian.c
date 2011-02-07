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

cst_lexicon *cmu_lex_init();

static cst_utterance* do_nothing(cst_utterance* u)
{
  return u;
}

static cst_val *add_lts_boundary_marks(const cst_val *l)
{
    cst_val *l1;
    const cst_val *v;
    l1 = cons_val(string_val("#"),NULL);
    for (v=l;v;v=val_cdr(v))
      {
        l1=cons_val(val_car(v),l1);
      }
    l1 = cons_val(string_val("#"),l1);
    l1 = val_reverse(l1);
    return l1;
}

cst_val* ru_lts_apply(const cst_val *input, const cst_lts_rewrites *rule)
{
  cst_val *l=add_lts_boundary_marks(input);
  cst_val *output=lts_rewrites(l, rule);
  delete_val(l);
  return output;
}

static const char* vpairs[][2]=
  {
    {"b","p"},
    {"bb","pp"},
    {"v","f"},
    {"vv","ff"},
    {"g","k"},
    {"gg","kk"},
    {"d","t"},
    {"dd","tt"},
    {"zh","sh"},
    {"z","s"},
    {"zz","ss"},
    {NULL,NULL}
  };

void ru_ff_register(cst_features *ffunctions);

static const char * const ru_punctuation = "\"'`.,:;!?-(){}[]";
static const char * const ru_prepunctuation = "-\"'`({[";
static const char * const ru_singlecharsymbols = "";
static const char * const ru_whitespace = " \t\n\r";

const char *russian_vpair(const char *ph)
{
  int i;
  for(i=0;vpairs[i][0];++i)
    {
      if(cst_streq(vpairs[i][0],ph))
        return vpairs[i][1];
      if(cst_streq(vpairs[i][1],ph))
        return vpairs[i][0];
    }
  return NULL;
}

void russian_init(cst_voice *v)
{
    feat_set(v->features,"utt_break",breakfunc_val(&ru_utt_break));
    feat_set(v->features,"phoneset",phoneset_val(&ru_phoneset));
    feat_set_string(v->features,"silence",ru_phoneset.silence);
    feat_set_string(v->features,"text_whitespace",ru_whitespace);
    feat_set_string(v->features,"text_postpunctuation",ru_punctuation);
    feat_set_string(v->features,"text_prepunctuation", ru_prepunctuation);
    feat_set_string(v->features,"text_singlecharsymbols", ru_singlecharsymbols);
    feat_set(v->features,"tokentowords_func",itemfunc_val(&ru_tokentowords));
    feat_set(v->features,"phrasing_cart",cart_val(&ru_phrasing_cart));
    feat_set(v->features,"lexical_insertion_func",uttfunc_val(&russian_lexical_insertion));
    feat_set(v->features,"phrasing_func",uttfunc_val(&russian_phrasify));
    feat_set(v->features,"intonation_func",uttfunc_val(&do_nothing));
    feat_set(v->features,"postlex_func",uttfunc_val(russian_postlex_function));
    feat_set(v->features,"duration_model_func",uttfunc_val(do_nothing));
    feat_set(v->features,"f0_model_func",uttfunc_val(do_nothing));
    feat_set(v->features,"cmu_lex",lexicon_val(cmu_lex_init()));
    ru_ff_register(v->ffunctions);
}
