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

cst_utterance *russian_phrasify(cst_utterance *u)
{
  cst_relation *r;
  cst_item *w, *p=NULL;
  const cst_val *v;
  cst_cart *phrasing_cart;
  r = utt_relation_create(u,"Phrase");
  phrasing_cart = val_cart(feat_val(u->features,"phrasing_cart"));
  for (p=NULL,w=relation_head(utt_relation(u,"Word")); w; w=item_next(w))
    {
      if (p == NULL)
	{
          p = relation_append(r,NULL);
          item_set_string(p,"name","B");
	}
      item_add_daughter(p,w);
      v = cart_interpret(w,phrasing_cart);
      if (!cst_streq(val_string(v),"NB"))
        {
          item_set_string(p,"name",val_string(v));
          p = NULL;
        }
    }
  return u;
}
