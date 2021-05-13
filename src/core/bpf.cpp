/* Copyright (C) 2017  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 2.1 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#include <stdexcept>
#include <locale>
#include <fstream>
#include "core/bpf.h"
#include "core/io.hpp"

extern "C"
{
  void bpf_init(BPF* bpf)
  {
    bpf->number=0;
    bpf->length=0;
    bpf->coefs=0;
}

  void bpf_clear(BPF* bpf)
  {
    if(bpf->coefs!=0)
      {
        for(unsigned int i=0;i<bpf->number;++i)
          {
            if(bpf->coefs[i]!=0)
              {
                delete[] bpf->coefs[i];
                bpf->coefs[i]=0;
              }
          }
        delete[] bpf->coefs;
        bpf->coefs=0;
      }
    bpf->number=0;
    bpf->length=0;
  }

  int bpf_load(BPF* bpf,const char* path)
  {
    std::ifstream f;
try
  {
    RHVoice::io::open_ifstream(f,path,false);
    if(!(f>>bpf->number))
      throw std::runtime_error("Cannot read number of filters");
    if(!(f>>bpf->length))
      throw std::runtime_error("Cannot read length of filters");
    bpf->coefs=new double*[bpf->number];
    for(unsigned int i=0;i<bpf->number;++i)
      bpf->coefs[i]=0;
    for(unsigned int i=0;i<bpf->number;++i)
      bpf->coefs[i]=new double[bpf->length];
    for(unsigned int i=0;i<bpf->number;++i)
      {
        for(unsigned int j=0;j<bpf->length;++j)
          {
            if(!(f>>bpf->coefs[i][j]))
              throw std::runtime_error("Cannot read filter coefficient");
}
}
    return 1;
}
 catch(std::exception& e)
   {
     bpf_clear(bpf);
     return 0;
}
}
}
