/* Copyright (C) 2019  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_EMOJI_HPP
#define RHVOICE_EMOJI_HPP

#include <memory>
#include "utf8.h"

namespace RHVoice
{
enum emoji_property_t
  {
   emoji_property_emoji=1,
   emoji_property_emoji_presentation=2,
   emoji_property_emoji_modifier=4,
   emoji_property_emoji_modifier_base=8,
   emoji_property_emoji_component=16,
emoji_property_extended_pictographic=32
  };

  struct emoji_char_t
  {
    utf8::uint32_t cp;
    unsigned int p;

    bool valid()
    {
      return ((p&emoji_property_emoji)||(p&emoji_property_emoji_component));
}

    bool operator<(const emoji_char_t& other) const
    {
      return (cp<other.cp);
}
  };

  emoji_char_t find_emoji_char(utf8::uint32_t c);

  const utf8::uint32_t emoji_presentation_selector=0xfe0f;
  const utf8::uint32_t zwj=0x200d;

  class emoji_scanner_state
  {
  public:
    emoji_scanner_state()
    {
}

    virtual ~emoji_scanner_state()
    {
}

    virtual bool can_end_emoji() const
    {
      return false;
}

    virtual std::unique_ptr<emoji_scanner_state> next(emoji_char_t c) const
    {
      return std::unique_ptr<emoji_scanner_state>();
}

  private:
    emoji_scanner_state(const emoji_scanner_state&);
    emoji_scanner_state& operator=(const emoji_scanner_state&);
  };

  class emoji_scanner
  {
  public:
    emoji_scanner();
    void reset();

    std::size_t get_result() const
    {
      return result;
}

    bool empty() const
    {
      return (length==0);
}

    bool process(utf8::uint32_t c);

  private:
    std::unique_ptr<emoji_scanner_state> state;
    std::size_t result;
    std::size_t length;
  };
}
#endif
