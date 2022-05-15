/* Copyright (C) 2022  Olga Yakovleva <olga@rhvoice.org> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 2 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#ifndef RHVOICE_MODEL_ANSWER_CACHE_H
#define RHVOICE_MODEL_ANSWER_CACHE_H
#ifdef __cplusplus
extern "C" {
  #endif

typedef struct
{
  void* impl;
  size_t index;
} RHVoice_model_answer_cache_t;
 
  size_t RHVoice_model_answer_cache_get(RHVoice_model_answer_cache_t cache, const char* label, size_t state);
  void RHVoice_model_answer_cache_put(RHVoice_model_answer_cache_t cache, const char* label, size_t state, size_t value);
  #ifdef __cplusplus
}
#endif
#endif
