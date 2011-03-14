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

#ifndef _LIB_H_
#define _LIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "RHVoice.h"
#include "ustring.h"
#include "mutex.h"
#include <flite.h>
#include <cst_lts_rewrites.h>
#include <unitypes.h>


  void russian_init(cst_utterance *u);

  cst_utterance* russian_phrasify(cst_utterance* u);
  cst_utterance *russian_textanalysis(cst_utterance *u);
  cst_utterance* russian_lexical_insertion(cst_utterance* u);
  cst_utterance *russian_postlex_function(cst_utterance *u);
  cst_utterance *russian_pause_insertion(cst_utterance *u);
  cst_utterance *create_hts_labels(cst_utterance *u);
  cst_utterance *hts_synth(cst_utterance *u);
  extern const cst_phoneset ru_phoneset;
  extern cst_lexicon *en_lex;
  char *ru_implode(const cst_val *l);
  cst_val* ru_lts_apply(const cst_val *input, const cst_lts_rewrites *rule);
  cst_val* ustring32_lts_apply(const ustring32_t u32,const cst_lts_rewrites *rule);
  const char *russian_vpair(const char *ph);

  extern const char *ru_proc_list[];
  extern const char *ru_enc_list[];
  extern const char *ru_h_enc_list[];

  void free_user_dict();
  const uint8_t* user_dict_lookup(const uint8_t* word);

  typedef enum {
    cs_sp=1 << 0,               /* space */
    cs_nl=1 << 1,               /* newline */
    cs_pr=1 << 2,               /* paragraph */
    cs_ws=cs_sp|cs_nl|cs_pr,    /* whitespace */
    cs_ll=1 << 3,               /* lowercase */
    cs_lu=1 << 4,               /* uppercase */
    cs_l=cs_ll|cs_lu,           /* letter */
    cs_d=1 << 5,                /* digit */
    cs_pd=1 << 6,               /* dash */
    cs_pq=1 << 7,               /* quote */
    cs_po=1 << 8,               /* other punctuation */
    cs_p=cs_pd|cs_pq|cs_po,     /* punctuation */
    cs_pi=1 << 9,               /* word-initial punctuation */
    cs_pf=1 << 10,              /* word-final punctuation */
    cs_lv=1 << 11,              /* vowel */
    cs_lc=1 << 12,              /* consonant */
    cs_ru=1 << 13,              /* Russian */
    cs_en=1 << 14,               /* English */
    cs_s=1 << 16,               /* symbol */
    cs_lw=1 << 17               /* a letter which can be a separate word */
  } character_subset;

  unsigned int classify_character(ucs4_t c);
  unsigned int classify_characters(const uint32_t *chars,size_t n);
  const char *character_name(ucs4_t c);

  cst_utterance *next_utt_from_message(RHVoice_message msg);
  int report_final_mark(RHVoice_message message,RHVoice_callback callback);

  extern MUTEX settings_mutex;

#ifdef __cplusplus
}
#endif

#endif
