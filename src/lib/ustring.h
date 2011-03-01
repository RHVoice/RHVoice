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

#ifndef USTRING_H
#define USTRING_H
#include <unitypes.h>

struct uchars32_s;
struct uchars8_s;
typedef struct uchars32_s *ustring32_t;
typedef struct uchars8_s *ustring8_t;

ustring32_t ustring32_alloc(size_t min_size);
ustring8_t ustring8_alloc(size_t min_size);
void ustring32_free(ustring32_t u);
void ustring8_free(ustring8_t u);
size_t ustring32_length(ustring32_t u);
size_t ustring8_length(ustring8_t u);
uint32_t *ustring32_str(ustring32_t u);
uint8_t *ustring8_str(ustring8_t u);
int ustring32_empty(ustring32_t u);
int ustring8_empty(ustring8_t u);
ustring32_t ustring32_push(ustring32_t u,ucs4_t c);
ustring8_t ustring8_push(ustring8_t u,ucs4_t c);
void ustring32_clear(ustring32_t u);
void ustring8_clear(ustring8_t u);
ustring32_t ustring32_reserve(ustring32_t u,size_t r);
ustring8_t ustring8_reserve(ustring8_t u,size_t r);
ustring8_t ustring32_substr8(ustring8_t u8,ustring32_t u32,size_t p,size_t l);
ustring32_t ustring32_assign8(ustring32_t u32,const uint8_t *u8);
ucs4_t ustring32_at(ustring32_t u,size_t i);
ustring32_t ustring32_set(ustring32_t u,size_t p,ucs4_t c);
#endif
