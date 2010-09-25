/* Copyright (C) 2010  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef CLASSFACTORYIMPL_H
#define CLASSFACTORYIMPL_H

#include <windows.h>

class CClassFactoryImpl : public IClassFactory
{
 public:
  CClassFactoryImpl();
  virtual ~CClassFactoryImpl();
  STDMETHOD_(ULONG,AddRef)();
  STDMETHOD_(ULONG,Release)();
  STDMETHOD(QueryInterface)(REFIID riid,void** ppv);
  STDMETHOD(CreateInstance)(IUnknown* pUnkOuter,REFIID riid,void** ppv);
  STDMETHOD(LockServer)(BOOL fLock);
 private:
  ULONG ref_count;
};

#endif
