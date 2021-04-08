/* Copyright (C) 2018  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include <new>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <cstring>
#include "core/question_matcher.h"

namespace
{

  bool prepare(RHVoice_parsed_label_string* p,const char* s)
  {
    RHVoice_parsed_label_string_init(p);
    std::fill(&(p->index[0]),&(p->index[0])+128,-1);
    if(s==0||s[0]==0)
      return false;
    p->label_string_length=std::strlen(s);
    if(p->label_string_length>RHVOICE_PARSED_LABEL_STRING_MAX_LENGTH)
      p->label_string_length=RHVOICE_PARSED_LABEL_STRING_MAX_LENGTH;
    try
      {
        p->label_string=new char[p->label_string_length+1];
        std::strncpy(p->label_string,s,p->label_string_length+1);
        p->label_string[p->label_string_length]=0;
        p->links=new short[p->label_string_length];
        std::fill(&(p->links[0]),&(p->links[0])+p->label_string_length,-1);
        return true;
}
    catch(const std::bad_alloc& e)
      {
        RHVoice_parsed_label_string_clear(p);
        return false;
}
}

  inline void set(RHVoice_parsed_label_string* p,short i,char c,short* last)
  {
    if(p->index[c]<0)
      {
        p->index[c]=i;
}
    else
      {
        p->links[last[c]]=i;
}
    last[c]=i;
}

}

extern "C"
{

void RHVoice_parsed_label_string_init(RHVoice_parsed_label_string* l)
{
  l->label_string=0;
  l->label_string_length=0;
  l->links=0;
}

  void RHVoice_parsed_label_string_clear(RHVoice_parsed_label_string* l)
  {
    if(l->links!=0)
      {
        delete[] l->links;
        l->links=0;
}
    if(l->label_string!=0)
      {
        delete[] l->label_string;
        l->label_string=0;
}
    l->label_string_length=0;
}

  int RHVoice_parse_label_string(const char* s,RHVoice_parsed_label_string* out)
  {
    if(!prepare(out,s))
      return 0;
    short i=0;
    char c=0;
    char pc=0;
    short last[128]={-1};
    while(i<out->label_string_length)
      {
        c=s[i];
        if((c>='a'&&c<='z')||(c>='A'&&c<='Z'))
          {
            if(pc=='/')
              set(out,i,c,&(last[0]));
          }
        else if((c>='0'&&c<='9')||c>127) ;
        else
          set(out,i,c,&(last[0]));
        pc=c;
        ++i;
}
    return 1;
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
    short j,k;
    for(j=l->index[q0[0]];j>=0;j=l->links[j])
      {
        k=l->label_string_length-j;
        if(k<n)
          return 0;
        if(strncmp(q0,l->label_string+j,n)==0)
          return 1;
      }
    return 0;
}

  int RHVoice_parsed_label_string_copy(const RHVoice_parsed_label_string* from,RHVoice_parsed_label_string* to)
  {
    if(from->label_string_length==0)
      {
        RHVoice_parsed_label_string_clear(to);
        return 1;
}
    char* str=0;
    try
      {
        str=new char[from->label_string_length+1];
}
    catch(const std::bad_alloc& e)
      {
        return 0;
}
    std::copy(from->label_string,from->label_string+from->label_string_length,str);
    str[from->label_string_length]=0;
    short* links=0;
    try
      {
        links=new short[from->label_string_length];
}
    catch(const std::bad_alloc& e)
      {
        delete[] str;
        return 0;
}
    std::copy(from->links,from->links+from->label_string_length,links);
    RHVoice_parsed_label_string_clear(to);
    to->label_string_length=from->label_string_length;
    to->label_string=str;
    to->links=links;
    std::copy(&(from->index[0]),&(from->index[0])+128,&(to->index[0]));
    return 1;
}
}

namespace RHVoice
{

  void parsed_label_string::parse(const char* s)
    {
      if(data.label_string_length!=0)
        throw std::logic_error(std::string("Already parsed"));
      if(!RHVoice_parse_label_string(s,&data))
        throw std::runtime_error("Failed to parse");
}

  void parsed_label_string::copy(const parsed_label_string& other)
    {
      if(!RHVoice_parsed_label_string_copy(&(other.data),&data))
        throw std::runtime_error("Cannot copy");
}
}
