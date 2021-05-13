/* Copyright (C) 2012  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_SAPI_IENUMSPOBJECTTOKENSIMPL_HPP
#define RHVOICE_SAPI_IENUMSPOBJECTTOKENSIMPL_HPP

#include <vector>
#include <windows.h>
#include <sapi.h>
#include <sapiddk.h>
#include <sperror.h>
#include <comdef.h>
#include <comip.h>

#include "tts_base.hpp"
#include "com.hpp"
#include "voice_attributes.hpp"
#include "voice_token.hpp"

namespace RHVoice
{
  namespace sapi
  {
    class __declspec(uuid("d7577808-7ade-4dea-a5b7-ee314d6ef3a1")) IEnumSpObjectTokensImpl:
      public IEnumSpObjectTokens, public tts_base
    {
    public:
      explicit IEnumSpObjectTokensImpl(bool initialize=true);

      STDMETHOD(Next)(ULONG celt,ISpObjectToken **pelt,ULONG *pceltFetched);
      STDMETHOD(Skip)(ULONG celt);
      STDMETHOD(Reset)();
      STDMETHOD(Clone)(IEnumSpObjectTokens **ppEnum);
      STDMETHOD(Item)(ULONG Index,ISpObjectToken **ppToken);
      STDMETHOD(GetCount)(ULONG *pulCount);

    protected:
      void* get_interface(REFIID riid)
      {
        return com::try_primary_interface<IEnumSpObjectTokens>(this,riid);
      }

    private:
      _COM_SMARTPTR_TYPEDEF(ISpObjectToken,__uuidof(ISpObjectToken));
      _COM_SMARTPTR_TYPEDEF(ISpObjectTokenInit,__uuidof(ISpObjectTokenInit));

      ISpObjectTokenPtr create_token(const voice_attributes& attr) const;

      std::wstring id;
      unsigned long index;
      std::vector<voice_attributes> sapi_voices;
    };
  }
}
#endif
