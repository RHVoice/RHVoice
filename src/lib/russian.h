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

#ifndef _RUSSIAN_H_
#define _RUSSIAN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <flite.h>
#include <cst_lts_rewrites.h>

  void russian_init(cst_voice *v);

  cst_utterance* russian_phrasify(cst_utterance* u);
  int ru_utt_break(cst_tokenstream *ts, const char *token, cst_relation *tokens);
  cst_val *ru_tokentowords(const cst_item *t);
  cst_utterance* russian_lexical_insertion(cst_utterance* u);
  cst_utterance *russian_postlex_function(cst_utterance *u);

  extern const cst_phoneset ru_phoneset;
  extern const cst_cart ru_phrasing_cart;

  cst_val* ru_lts_apply(const cst_val *input, const cst_lts_rewrites *rule);
  const char *russian_vpair(const char *ph);

  typedef struct ru_user_dict_struct {
    int size;
    char*** entries;
  } ru_user_dict;

  ru_user_dict* ru_user_dict_load(const char*path);
  void ru_user_dict_free(ru_user_dict* dict);
  const char** ru_user_dict_lookup(const ru_user_dict* dict,const char* word);

#ifdef __cplusplus
}
#endif

#endif
