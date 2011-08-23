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
#include <cst_ffeatures.h>

static const cst_val *ph_csoft(const cst_item *p)
{
  return phone_feature(item_phoneset(p),item_name(p),"csoft");
}

DEF_STATIC_CONST_VAL_STRING(val_string_content,"content");
DEF_STATIC_CONST_VAL_STRING(val_string_proc,"proc");
DEF_STATIC_CONST_VAL_STRING(val_string_enc,"enc");

static const cst_val *gpos(const cst_item *word)
{
  const char *name=item_feat_string(word,"name");
  if(item_feat_present(word,"my_gpos"))
    return item_feat(word,"my_gpos");
  if(item_prev(item_as(word,"Token"))&&cst_member_string(name,ru_h_enc_list))
    return &val_string_enc;
  if(item_next(item_as(word,"Word"))&&cst_member_string(name,ru_proc_list))
    return &val_string_proc;
  if(item_prev(item_as(word,"Word"))&&cst_member_string(name,ru_enc_list))
    return &val_string_enc;
  return &val_string_content;
}

static const cst_val *has_vpair(const cst_item *s)
{
  const char *name=item_feat_string(s,"name");
  const char *pair=russian_vpair(name);
  if(pair==NULL)
    return VAL_STRING_0;
  return VAL_STRING_1;
}

void ru_ff_register(cst_features *ffunctions)
{
  basic_ff_register(ffunctions);
  ff_register(ffunctions, "ph_csoft",ph_csoft);
  ff_register(ffunctions, "gpos",gpos);
  ff_register(ffunctions, "lisp_ru_has_vpair",has_vpair);
}
