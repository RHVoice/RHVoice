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

#include <algorithm>
#include "core/emoji.hpp"

namespace RHVoice
{
  namespace
  {
#include "emoji_data.cpp"

    const emoji_char_t* end_emoji_char=&emoji_chars[0]+num_emoji_chars;

    const utf8::uint32_t ri_a=0x1f1e6;
    const utf8::uint32_t ri_z=0x1f1ff;
    const utf8::uint32_t keycap=0x20e3;
    const utf8::uint32_t tag_space=0xe0020;
    const utf8::uint32_t cancel_tag=0xe007f;

    inline bool is_regional_indicator(emoji_char_t c)
    {
      return (c.cp>=ri_a&&c.cp<=ri_z);
}

    inline bool is_key_char(emoji_char_t c)
    {
      return ((c.cp>='0'&&c.cp<='9')||c.cp=='*'||c.cp=='#');
}

    inline bool is_tag_spec_char(emoji_char_t c)
    {
      return (c.cp>=tag_space&&c.cp<cancel_tag);
}

    class initial_scanner_state: public emoji_scanner_state
    {
    public:
      std::unique_ptr<emoji_scanner_state> next(emoji_char_t c) const;
    };

    class first_ri_scanner_state: public emoji_scanner_state
    {
    public:
      std::unique_ptr<emoji_scanner_state> next(emoji_char_t c) const;
    };

    class second_ri_scanner_state: public emoji_scanner_state
    {
    public:
      bool can_end_emoji() const;
    };

    class first_keycap_seq_scanner_state: public emoji_scanner_state
    {
    public:
      std::unique_ptr<emoji_scanner_state> next(emoji_char_t c) const;
    };

    class second_keycap_seq_scanner_state: public emoji_scanner_state
    {
    public:
      std::unique_ptr<emoji_scanner_state> next(emoji_char_t c) const;
    };

    class third_keycap_seq_scanner_state: public emoji_scanner_state
    {
    public:
      bool can_end_emoji() const;
    };

    class tag_spec_scanner_state: public emoji_scanner_state
    {
    public:
      std::unique_ptr<emoji_scanner_state> next(emoji_char_t c) const;
    };

    class tag_term_scanner_state: public emoji_scanner_state
    {
    public:
      bool can_end_emoji() const;
    };

    class zwj_element_scanner_state: public emoji_scanner_state
    {
    public:
      std::unique_ptr<emoji_scanner_state> next(emoji_char_t c) const;
      bool can_end_emoji() const;

    protected:
      bool first;
      zwj_element_scanner_state(bool f):
        first(f)
      {
}
    };

    class emoji_char_scanner_state: public zwj_element_scanner_state
    {
    public:
      emoji_char_scanner_state(emoji_char_t c,bool f):
        zwj_element_scanner_state(f),
        chr(c)
      {
}

      std::unique_ptr<emoji_scanner_state> next(emoji_char_t c) const;

    private:
      emoji_char_t chr;
    };

    class mod_seq_scanner_state: public zwj_element_scanner_state
    {
    public:
      mod_seq_scanner_state(bool f):
        zwj_element_scanner_state(f)
      {
}
    };

    class pres_seq_scanner_state: public zwj_element_scanner_state
    {
    public:
      pres_seq_scanner_state(bool f):
        zwj_element_scanner_state(f)
      {
}
    };

    class zwj_scanner_state: public emoji_scanner_state
    {
    public:
      std::unique_ptr<emoji_scanner_state> next(emoji_char_t c) const;
    };

    std::unique_ptr<emoji_scanner_state> initial_scanner_state::next(emoji_char_t c) const
    {
      std::unique_ptr<emoji_scanner_state> res;
      if(is_regional_indicator(c))
        res.reset(new first_ri_scanner_state);
      else if(c.p&emoji_property_emoji)
        {
          if(is_key_char(c))
            res.reset(new first_keycap_seq_scanner_state);
          else
            res.reset(new emoji_char_scanner_state(c,true));
      }
      return res;
}

    std::unique_ptr<emoji_scanner_state> zwj_element_scanner_state::next(emoji_char_t c) const
    {
      std::unique_ptr<emoji_scanner_state> res;
      if(c.cp==zwj)
        res.reset(new zwj_scanner_state);
      else if(first&&is_tag_spec_char(c))
        res.reset(new tag_spec_scanner_state);
      return res;
}

    bool zwj_element_scanner_state::can_end_emoji() const
    {
      return true;
}

    std::unique_ptr<emoji_scanner_state> emoji_char_scanner_state::next(emoji_char_t c) const
    {
      std::unique_ptr<emoji_scanner_state> res(zwj_element_scanner_state::next(c));
      if(res.get()!=0)
        return res;
      if(c.cp==emoji_presentation_selector)
        res.reset(new pres_seq_scanner_state(first));
      else if((c.p&emoji_property_emoji_modifier)&&(chr.p&emoji_property_emoji_modifier_base))
        res.reset(new mod_seq_scanner_state(first));
      return res;
}

    std::unique_ptr<emoji_scanner_state> zwj_scanner_state::next(emoji_char_t c) const
    {
      std::unique_ptr<emoji_scanner_state> res;
      if((c.p&emoji_property_emoji)&&!is_key_char(c))
        res.reset(new emoji_char_scanner_state(c,false));
      return res;
}

    std::unique_ptr<emoji_scanner_state> first_ri_scanner_state::next(emoji_char_t c) const
    {
      std::unique_ptr<emoji_scanner_state> res;
      if(is_regional_indicator(c))
        res.reset(new second_ri_scanner_state);
      return res;
}

    bool second_ri_scanner_state::can_end_emoji() const
    {
      return true;
}

    std::unique_ptr<emoji_scanner_state> first_keycap_seq_scanner_state::next(emoji_char_t c) const
    {
      std::unique_ptr<emoji_scanner_state> res;
      if(c.cp==emoji_presentation_selector)
        res.reset(new second_keycap_seq_scanner_state);
      return res;
}

    std::unique_ptr<emoji_scanner_state> second_keycap_seq_scanner_state::next(emoji_char_t c) const
    {
      std::unique_ptr<emoji_scanner_state> res;
      if(c.cp==keycap)
        res.reset(new third_keycap_seq_scanner_state);
      return res;
}

    bool third_keycap_seq_scanner_state::can_end_emoji() const
    {
      return true;
}

    std::unique_ptr<emoji_scanner_state> tag_spec_scanner_state::next(emoji_char_t c) const
    {
      std::unique_ptr<emoji_scanner_state> res;
      if(c.cp==cancel_tag)
        res.reset(new tag_term_scanner_state);
      else if(is_tag_spec_char(c))
        res.reset(new tag_spec_scanner_state);
      return res;
}

    bool tag_term_scanner_state::can_end_emoji() const
    {
      return true;
}

}

    emoji_char_t find_emoji_char(utf8::uint32_t c)
    {
      emoji_char_t tmp={c,0};
      const emoji_char_t* ptr=std::lower_bound(&emoji_chars[0],end_emoji_char,tmp);
      if(ptr!=end_emoji_char&&ptr->cp==c)
        tmp=*ptr;
      return tmp;
}

  emoji_scanner::emoji_scanner()
  {
    reset();
}

  void emoji_scanner::reset()
  {
    result=0;
    length=0;
    state.reset(new initial_scanner_state);
}

  bool emoji_scanner::process(utf8::uint32_t cp)
  {
    emoji_char_t c=find_emoji_char(cp);
    if(!c.valid())
      return false;
    std::unique_ptr<emoji_scanner_state> new_state=state->next(c);
    if(new_state.get()==0)
      return false;
    state = std::move(new_state);
    ++length;
    if(state->can_end_emoji())
      result=length;
    return true;
}
}
