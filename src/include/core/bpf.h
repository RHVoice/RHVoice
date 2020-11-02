/* Copyright (C) 2017  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#ifndef RHVOICE_BPF_H
#define RHVOICE_BPF_H

#include "api.hpp"
#include "abstraction_layer.h"

#ifdef RHVOICE_FILTER_MANAGER_EXPORTS
#    define RHVOICE_FILTER_MANAGER_API RHVOICE_EXPORT_API
#else
#    define RHVOICE_FILTER_MANAGER_API RHVOICE_IMPORT_API
#endif

#ifdef __cplusplus
extern "C" {
  #endif

typedef struct
{
  unsigned int number;
  unsigned int length;
  double** coefs;
} BPF;

  RHVOICE_FILTER_MANAGER_API void bpf_init(BPF* bpf);
  RHVOICE_FILTER_MANAGER_API void bpf_clear(BPF* bpf);
  RHVOICE_FILTER_MANAGER_API int bpf_load(BPF* bpf, const PathCharT * path);
#ifdef __cplusplus
}
#endif
#endif
