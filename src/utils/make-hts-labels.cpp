/* Copyright (C) 2012, 2013  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 2 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#include <stdexcept>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <iterator>

#ifdef WITH_CLI11
	#include <CLI/CLI.hpp>
#else
	#include "tclap/CmdLine.h"
#endif
#include "core/str.hpp"
#include "core/path.hpp"
#include "core/engine.hpp"
#include "core/document.hpp"
#include "core/hts_labeller.hpp"

using namespace RHVoice;

namespace
{
  std::list<std::string> list_lab_files(const std::string& p)
  {
    std::list<std::string> result;
    for(path::directory dir(p);!dir.done();dir.next())
      {
        std::string name(dir.get());
        if(str::endswith(name,".lab"))
          {
            std::string full_path(path::join(p,name));
            if(path::isfile(full_path))
              result.push_back(name);
          }
      }
    result.sort();
    return result;
  }

  void output_labels(const utterance& utt,const std::string& file_path)
  {
    std::ofstream f(file_path.c_str());
    if(!f.is_open())
      throw std::runtime_error("Cannot open an output file");
    const hts_labeller& labeller=utt.get_language().get_hts_labeller();
    const relation& seg_rel=utt.get_relation("Segment");
    for(relation::const_iterator seg_iter(seg_rel.begin());seg_iter!=seg_rel.end();++seg_iter)
      {
        f << seg_iter->eval("start",value()) << " ";
        f << seg_iter->eval("end",value()) << " ";
        f << labeller.eval_segment_label(*seg_iter) << std::endl;
      }
  }

  void rephrase(utterance& utt)
  {
    relation& phrase_rel=utt.get_relation("Phrase");
    phrase_rel.clear();
    relation& word_rel=utt.get_relation("Word");
    relation::iterator word_iter(word_rel.begin());
    if(word_iter!=word_rel.end())
      {
        phrase_rel.append();
        do
          {
            phrase_rel.last().append_child(*word_iter);
            const item& last_seg=word_iter->as("Transcription").last_child().as("Segment");
            ++word_iter;
            if(last_seg.has_next()&&(word_iter!=word_rel.end()))
              {
                const item& seg=last_seg.next();
                if(seg.get("name").as<std::string>()=="pau")
                  phrase_rel.append();
              }
          }
        while(word_iter!=word_rel.end());
      }
  }

  void load_mono_labels(utterance& utt,const std::string& file_path)
  {
    std::cout << file_path << std::endl;
    relation& seg_rel=utt.get_relation("Segment");
    relation::iterator seg_iter(seg_rel.begin());
    unsigned int start,end;
    std::string phone,name;
    std::ifstream f(file_path.c_str());
    while(seg_iter!=seg_rel.end())
      {
        if(!(f >> start >> end >> phone))
          throw std::runtime_error("Error while reading a label file");
        name=seg_iter->get("name").as<std::string>();
        if((name=="pau")&&(phone!="pau"))
          {
            item& seg=*seg_iter;
            ++seg_iter;
            if(seg_iter==seg_rel.end())
              throw std::runtime_error("Label mismatch");
            seg.remove();
            name=seg_iter->get("name").as<std::string>();
          }
        if(name==phone)
          {
            seg_iter->set("start",start);
            seg_iter->set("end",end);
            ++seg_iter;
          }
        else if(phone=="pau")
          {
            item& seg=seg_iter->prepend();
            seg.set("name",phone);
            seg.set("start",start);
            seg.set("end",end);
          }
        else
          throw std::runtime_error("Label mismatch: expected "+name+", found "+phone);
      }
    rephrase(utt);
  }
}

#ifdef WITH_CLI11
	typedef CLI::App AppT;
	#define GET_CLI_PARAM_VALUE(NAME) (NAME ## Stor)
	#define CHECK_CLI_PARAM_STR_VALUE_SET(NAME) (! (NAME ## Stor.empty()))
#else
	typedef TCLAP::CmdLine AppT;
	#define GET_CLI_PARAM_VALUE(NAME) (NAME).getValue()
	#define CHECK_CLI_PARAM_STR_VALUE_SET(NAME) (NAME).isSet()
#endif

int main(int argc,const char* argv[])
{
  try
    {
      AppT cmd("Write hts labels for a list of sentences from SSML document");

#ifdef WITH_CLI11
      std::string inpath_argStor {"text.ssml"};
      auto inpath_arg = cmd.add_option("input",inpath_argStor,"input file")->required();
      std::string outpath_argStor {"lab"};
      auto outpath_arg = cmd.add_option("output",outpath_argStor,"output directory")->required();

      auto wtfGroup = cmd.add_option_group("WTF", "WTF");
      std::string labpath_argStor;
      auto labpath_arg = wtfGroup->add_option("-l,--lab",labpath_argStor,"the path to the mono labels")->required();
      std::string prefix_argStor {"test"};
      auto prefix_arg = wtfGroup->add_option("-p,--prefix",prefix_argStor,"output file names will start with this prefix")->excludes(labpath_arg);
      wtfGroup->require_option(1);
#else
      TCLAP::UnlabeledValueArg<std::string> inpath_arg("input","input file",true,"text.ssml","infile",cmd);
      TCLAP::UnlabeledValueArg<std::string> outpath_arg("output","output directory",true,"lab","outdir",cmd);
      TCLAP::ValueArg<std::string> labpath_arg("l","lab","the path to the mono labels",true,"lab","path");
      TCLAP::ValueArg<std::string> prefix_arg("p","prefix","output file names will start with this prefix",true,"test","string");
      cmd.xorAdd(labpath_arg,prefix_arg);
#endif

#ifdef WITH_CLI11
     try{
#endif
      cmd.parse(argc,argv);
#ifdef WITH_CLI11
      }catch (const CLI::ParseError &e) {
        return cmd.exit(e);
      }
#endif

      std::shared_ptr<engine> eng(new engine);
      std::ifstream f_in(GET_CLI_PARAM_VALUE(inpath_arg).c_str());
      if(!f_in.is_open())
        throw std::runtime_error("Cannot open the input file");
      std::istreambuf_iterator<char> text_start(f_in);
      std::istreambuf_iterator<char> text_end;
      std::unique_ptr<document> doc=document::create_from_ssml(eng,text_start,text_end);
      document::iterator sentence_iter=doc->begin();
      if(CHECK_CLI_PARAM_STR_VALUE_SET(labpath_arg))
        {
          std::list<std::string> fnames=list_lab_files(GET_CLI_PARAM_VALUE(labpath_arg));
          for(std::list<std::string>::const_iterator it(fnames.begin());it!=fnames.end();++it)
            {
              if(sentence_iter==doc->end())
                throw std::runtime_error("Sentence count mismatch");
              std::unique_ptr<utterance> utt=sentence_iter->create_utterance(sentence_position_single);
              load_mono_labels(*utt,path::join(GET_CLI_PARAM_VALUE(labpath_arg),*it));
              output_labels(*utt,path::join(GET_CLI_PARAM_VALUE(outpath_arg),*it));
              ++sentence_iter;
            }
        }
      else
        {
          unsigned int index=1;
          for(;sentence_iter!=doc->end();++sentence_iter)
            {
              std::ostringstream s;
              s << GET_CLI_PARAM_VALUE(prefix_arg);
              s << "_";
              s << index;
              s << ".lab";
              std::unique_ptr<utterance> utt=sentence_iter->create_utterance(sentence_position_single);
              output_labels(*utt,path::join(GET_CLI_PARAM_VALUE(outpath_arg),s.str()));
              ++index;
            }
        }
      return 0;
    }
  catch(const std::exception& e)
    {
      std::cerr << e.what() << std::endl;
      return -1;
    }
}
