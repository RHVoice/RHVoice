/* Copyright (C) 2012, 2021  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <iterator>
#include <utility>
#include "rapidxml/rapidxml.hpp"
#include "utf.hpp"
#include "str.hpp"
#include "tts_markup.hpp"

#ifndef RHVOICE_XML_HPP
#define RHVOICE_XML_HPP
namespace RHVoice
{
  class document;

  namespace xml
  {
    template<typename forward_iterator>
    class text_iterator: public std::iterator<std::forward_iterator_tag,const utf8::uint32_t>
    {
    public:
      text_iterator():
        code_point('\0')
      {
      }

      text_iterator(const forward_iterator& it,const forward_iterator& range_start_,const forward_iterator& range_end_):
        code_point('\0'),
        start(it),
        end(it),
        range_start(range_start_),
        range_end(range_end_)
      {
        ++(*this);
      }

      const utf8::uint32_t& operator*() const
      {
        return code_point;
      }

      bool operator==(const text_iterator& other) const
      {
        return ((range_start==other.range_start)&&(range_end==other.range_end)&&(start==other.start));
      }

      bool operator!=(const text_iterator& other) const
      {
        return !(*this==other);
      }

      text_iterator& operator++();

      text_iterator operator++(int)
      {
        text_iterator tmp=*this;
        ++(*this);
        return tmp;
      }

      forward_iterator base() const
      {
        return start;
      }

      std::size_t offset() const
      {
        return std::distance(range_start,start);
      }

    private:
      typedef typename std::iterator_traits<forward_iterator>::value_type char_type;
      bool translate(const std::basic_string<char_type>& s,utf8::uint32_t& r);

      utf8::uint32_t code_point;
      forward_iterator start,end,range_start,range_end;
    };

    template<typename forward_iterator>
    text_iterator<forward_iterator>& text_iterator<forward_iterator>::operator++()
    {
      if(end==range_end)
        start=end;
      else
        {
          forward_iterator tmp;
          if(*end=='&')
            {
              start=end;
              ++end;
              if(end!=range_end)
                {
                  tmp=std::find(end,range_end,';');
                  if((tmp!=end)&&(tmp!=range_end)&&(translate(std::basic_string<char_type>(end,tmp),code_point)))
                    end=tmp+1;
                }
            }
          else
            {
              tmp=end;
              code_point=utf::next(tmp,range_end);
              start=end;
              end=tmp;
          }
        }
      return *this;
    }

    template<typename forward_iterator>
    bool text_iterator<forward_iterator>::translate(const std::basic_string<char_type>& s,utf8::uint32_t& r)
    {
      if(s.size()>1)
        {
          utf8::uint32_t cp;
          std::basic_istringstream<char_type> in;
          switch(s[0])
            {
            case 'l':
              if((s.size()==2)&&(s[1]=='t'))
                {
                  r='<';
                  return true;
                }
              break;
            case 'g':
              if((s.size()==2)&&(s[1]=='t'))
                {
                  r='>';
                  return true;
                }
              break;
            case 'a':
              if((s.size()==3)&&(s[1]=='m')&&(s[2]=='p'))
                {
                  r='&';
                  return true;
                }
              else
                {
                  if((s.size()==4)&&(s[1]=='p')&&(s[2]=='o')&&(s[3]=='s'))
                    {
                      r='\'';
                      return true;
                    }
                }
              break;
            case 'q':
              if((s.size()==4)&&(s[1]=='u')&&(s[2]=='o')&&(s[3]=='t'))
                {
                  r='\"';
                  return true;
                }
              break;
            case '#':
              if(s[1]=='x')
                {
                  if(s.size()>2)
                    {
                      in.str(s.substr(2));
                      in.setf(std::ios::hex,std::ios::basefield);
                    }
                }
              else
                in.str(s.substr(1));
              if((in>>cp)&&utf::is_valid(cp))
                {
                  r=cp;
                  return true;
                }
              break;
            default:
              break;
            }
        }
      return false;
    }

    template<typename ch>
    inline const rapidxml::xml_attribute<ch>* find_attribute(const rapidxml::xml_node<ch>* node,const char* name)
    {
      std::basic_string<ch> str_name(name,name+std::strlen(name));
      return node->first_attribute(str_name.data(),str_name.size());
    }

    template<typename ch>
    bool get_attribute_value_range(const rapidxml::xml_node<ch>* node,const char* name, text_iterator<const ch*>& start, text_iterator<const ch*>& end)
    {
      if(const rapidxml::xml_attribute<ch>* attr=find_attribute(node,name))
        {
          std::size_t value_size=attr->value_size();
          if(value_size>0)
            {
              const ch* value_start=attr->value();
              start=text_iterator<const ch*>(value_start,value_start,value_start+value_size);
              end=text_iterator<const ch*>(value_start+value_size,value_start,value_start+value_size);
              return true;
            }
        }
      return false;
    }

    template<typename ch>
    std::string get_attribute_value(const rapidxml::xml_node<ch>* node,const char* name)
    {
      std::string result;
      text_iterator<const ch*> start, end;
      auto r=get_attribute_value_range(node, name, start, end);
      if(r)
        std::copy(start, end, str::utf8_inserter(std::back_inserter(result)));
      return result;
    }

    template<typename ch>
    struct handler_args
    {
      const ch* const xml_start;
      const std::size_t xml_size;
      document& target_document;
      tts_markup tts_markup_info;
      const rapidxml::xml_node<ch>* node;

      handler_args(const ch* xml_start_,std::size_t xml_size_,document& target_document_):
        xml_start(xml_start_),
        xml_size(xml_size_),
        target_document(target_document_),
        node(0)
      {
      }

    private:
      handler_args(const handler_args&);
      handler_args& operator=(const handler_args&);
    };

    template<typename ch>
    class element_handler
    {
    public:
      virtual ~element_handler() = default;

      const std::basic_string<ch>& get_name() const
      {
        return name;
      }

      virtual bool enter(handler_args<ch>& args)
      {
        return true;
      }

      virtual void leave(handler_args<ch>& args)
      {
      }

    protected:
      element_handler(const char* name_):
      name(name_,name_+std::strlen(name_))
      {
      }

    private:
      element_handler(const element_handler&);
      element_handler& operator=(const element_handler&);

      std::basic_string<ch> name;
    };

    template<typename ch>
    class text_handler
    {
    public:
      virtual ~text_handler() = default;

      virtual void process(handler_args<ch>& args);
    };

    template<typename ch>
    void text_handler<ch>::process(handler_args<ch>& args)
    {
      if(args.node->type()==rapidxml::node_data)
        {
          text_iterator<const ch*> text_start(args.node->value(),args.xml_start,args.xml_start+args.xml_size);
          text_iterator<const ch*> text_end(args.node->value()+args.node->value_size(),args.xml_start,args.xml_start+args.xml_size);
          args.target_document.add_text(text_start,text_end,args.tts_markup_info);
        }
      else
        {
          utf::text_iterator<const ch*> text_start(args.node->value(),args.xml_start,args.xml_start+args.xml_size);
          utf::text_iterator<const ch*> text_end(args.node->value()+args.node->value_size(),args.xml_start,args.xml_start+args.xml_size);
          args.target_document.add_text(text_start,text_end,args.tts_markup_info);
        }
    }

    template<typename ch>
    class parser
    {
    private:
      typedef std::map<std::basic_string<ch>,element_handler<ch>*> element_handler_table;

    public:
      parser():
        text_handler_ptr(0)
      {
      }

      void add_element_handler(element_handler<ch>& handler)
      {
        #ifdef _MSC_VER
        element_handlers.insert(element_handler_table::value_type(handler.get_name(),&handler));
        #else
        element_handlers.insert(typename element_handler_table::value_type(handler.get_name(),&handler));
        #endif
      }

      void set_text_handler(text_handler<ch>& handler)
      {
        text_handler_ptr=&handler;
      }

      template<typename input_iterator>
      void parse(const input_iterator& text_start,const input_iterator& text_end,document& target_document);

    private:
      element_handler<ch>* find_element_handler(const rapidxml::xml_node<ch>* node)
      {
        std::basic_string<ch> name(node->name(),node->name_size());
        #ifdef _MSC_VER
        element_handler_table::iterator it(element_handlers.find(name));
        #else
        typename element_handler_table::iterator it(element_handlers.find(name));
        #endif
        return ((it==element_handlers.end())?0:(it->second));
      }

      element_handler_table element_handlers;
      text_handler<ch>* text_handler_ptr;
    };

    template<typename ch> template<typename input_iterator>
    void parser<ch>::parse(const input_iterator& text_start,const input_iterator& text_end,document& target_document)
    {
      if(text_start==text_end)
        return;
      std::vector<ch> xml_text(text_start,text_end);
      std::replace(xml_text.begin(),xml_text.end(),'\0',' ');
      xml_text.push_back('\0');
      handler_args<ch> args(&xml_text[0],xml_text.size()-1,target_document);
      rapidxml::xml_document<ch> xml_doc;
      xml_doc.template parse<rapidxml::parse_non_destructive>(&xml_text[0]);
      const rapidxml::xml_node<ch>* prev_node=0;
      const rapidxml::xml_node<ch> *next_node,*first_child,*last_child;
      element_handler<ch>* elem_handler;
      const rapidxml::xml_node<ch>* root_node=xml_doc.first_node();
      args.node=root_node;
      while(args.node)
        {
          if(args.node==root_node)
            next_node=0;
          else
            {
              next_node=args.node->next_sibling();
              if(!next_node)
                next_node=args.node->parent();
            }
          switch(args.node->type())
            {
            case rapidxml::node_element:
              elem_handler=find_element_handler(args.node);
              if(elem_handler)
                {
                  first_child=args.node->first_node();
                  if(first_child)
                    {
                      last_child=args.node->last_node();
                      if(prev_node==last_child)
                        elem_handler->leave(args);
                      else
                        {
                          if(elem_handler->enter(args))
                            next_node=first_child;
                          else
                            elem_handler->leave(args);
                        }
                    }
                  else
                    {
                      elem_handler->enter(args);
                      elem_handler->leave(args);
                    }
                }
              break;
            case rapidxml::node_data:
            case rapidxml::node_cdata:
              if(text_handler_ptr)
                text_handler_ptr->process(args);
              break;
            default:
              break;
            }
          prev_node=args.node;
          args.node=next_node;
        }
    }
  }
}
#endif
