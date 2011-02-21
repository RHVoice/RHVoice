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

extern cst_val *ru_gpos;
DEF_STATIC_CONST_VAL_STRING(val_string_content,"content");

static const cst_val *gpos(const cst_item *word)
{
  const char *w=item_feat_string(word,"name");
  const cst_val *l;
  if(!item_next(item_as(word,"Word"))&&!item_prev(item_as(word,"Word")))
    return &val_string_content;
  for (l=ru_gpos; l; l=val_cdr(l))
    {
      if(val_member_string(w,val_cdr(val_car(l))))
        return val_car(val_car(l));
    }
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
