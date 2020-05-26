/* Copyright (C) 2012  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 2 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <stdexcept>
#include <string>
#include <iostream>
#include <fstream>
#include <iterator>
#include "tclap/CmdLine.h"
#include "core/smart_ptr.hpp"
#include "core/engine.hpp"
#include "core/document.hpp"

using namespace RHVoice;

int main(int argc,const char* argv[])
{
  try
    {
      TCLAP::CmdLine cmd("Transcribe sentences in SSML document");
      TCLAP::UnlabeledValueArg<std::string> inpath_arg("input","input file",true,"text.ssml","infile",cmd);
      TCLAP::UnlabeledValueArg<std::string> outpath_arg("output","output file",true,"transcription.txt","outfile",cmd);
      TCLAP::ValueArg<std::string> boundary_arg("b","boundary","word boundary marker",false,"","string",cmd);
      cmd.parse(argc,argv);
      smart_ptr<engine> eng(new engine);
      std::ifstream f_in(inpath_arg.getValue().c_str());
      if(!f_in.is_open())
        throw std::runtime_error("Cannot open the input file");
      std::istreambuf_iterator<char> text_start(f_in);
      std::istreambuf_iterator<char> text_end;
      std::unique_ptr<document> doc=document::create_from_ssml(eng,text_start,text_end);
      std::ofstream f_out(outpath_arg.getValue().c_str());
      if(!f_out.is_open())
        throw std::runtime_error("Cannot open the output file");
      for(document::iterator it(doc->begin());it!=doc->end();++it)
        {
          std::unique_ptr<utterance> utt=it->create_utterance(sentence_position_single);
          const relation& seg_rel=utt->get_relation("Segment");
          for(relation::const_iterator seg_iter(seg_rel.begin());seg_iter!=seg_rel.end();++seg_iter)
            {
              f_out << seg_iter->get("name") << " ";
              if((!boundary_arg.getValue().empty())&&
                 (seg_iter->in("Transcription"))&&
                 (!seg_iter->as("Transcription").has_next())&&
                 (seg_iter->eval("n.name").as<std::string>()!="pau"))
                f_out << boundary_arg.getValue() << " ";
            }
          f_out << std::endl;
        }
      return 0;
    }
  catch(const std::exception& e)
    {
      std::cerr << e.what() << std::endl;
      return -1;
    }
}
