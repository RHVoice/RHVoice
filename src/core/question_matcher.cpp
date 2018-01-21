/* Copyright (C) 2018  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <algorithm>
#include <cstring>
#include "core/question_matcher.h"

extern "C"
{

  void RHVoice_parse_label_string(const char* s,RHVoice_parsed_label_string* out)
  {
    out->label_string[0]=0;
    out->label_string_length=0;
    std::fill(&(out->index[0][0]),&(out->index[0][0])+(sizeof(out->index)/sizeof(out->index[0][0])),0);
    if(s==0)
      return;
    short i=0;
    char c=0;
    char pc=0;
    while(s[i]!=0&&i<RHVOICE_PARSED_LABEL_STRING_MAX_LENGTH)
      {
        c=s[i];
        out->label_string[i]=c;
        if((c>='a'&&c<='z')||(c>='A'&&c<='Z'))
          {
            if(pc=='/')
              {
                ++(out->index[c][0]);
                out->index[c][out->index[c][0]]=i;
              }
          }
        else if((c>='0'&&c<='9')||c>127) ;
        else
          {
            ++(out->index[c][0]);
            out->index[c][out->index[c][0]]=i;
}
        pc=c;
        ++i;
}
    out->label_string[i]=0;
    out->label_string_length=i;
}

  int RHVoice_question_match(const RHVoice_parsed_label_string* l,const char* q)
  {
    if(q==0)
      return 0;
    std::size_t n=std::strlen(q);
    if(n==0)
      return 0;
    const char* q0=q;
    bool prefix=(q0[0]!='*');
    if(!prefix)
      {
        ++q0;
        --n;
        if(n==0)
          return 1;
}
    bool suffix=(q0[n-1]!='*');
    if(!suffix)
      {
        --n;
        if(n==0)
          return 1;
}
    if(n>l->label_string_length)
      return 0;
    if(prefix)
      {
        if(strncmp(q0,l->label_string,n)==0)
          return 1;
        else
          return 0;
}
    if(suffix)
      {
        if(strncmp(q0,l->label_string+(l->label_string_length-n),n)==0)
          return 1;
        else
          return 0;
}
    if(q0[0]>127)
      return 0;
    if(q0[0]=='/'&&n>1&&((q0[1]>='A'&&q0[1]<='Z')||(q0[1]>='a'&&q0[1]<='z')))
      {
        ++q0;
        --n;
}
    const short m=l->index[q0[0]][0];
    short i,j,k;
    for(i=1;i<=m;++i)
      {
        j=l->index[q0[0]][i];
        k=l->label_string_length-j;
        if(k<n)
          return 0;
        if(strncmp(q0,l->label_string+j,n)==0)
          return 1;
}
    return 0;
}

}
