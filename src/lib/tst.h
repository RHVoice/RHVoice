/* Copyright (C) 2011  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef TST_H
#define TST_H

#include <unitypes.h>

struct tst_s;
typedef struct tst_s *tst;
typedef void (*tst_free_value_func)(void *value);

tst tst_create(int store_values,tst_free_value_func free_value_func);
void tst_free(tst tree);
int tst_add(tst tree,const uint8_t *key,void *value);
void tst_build(tst tree);
int tst_search(tst tree,const uint8_t *key,void **pvalue);

#endif
