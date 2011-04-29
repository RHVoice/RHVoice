/* Copyright (C) 2009, 2010, 2011  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include <stdio.h>
#include "lib.h"

cst_utterance *create_hts_labels(cst_utterance *u)
{
  const int size=1024;
  char label[size];
  char *ptr=label;
  cst_item *s,*ps,*pps,*ns,*nns,*ss,*tmp,*ssyl,*sw,*syl,*w,*phr;
  sw=NULL;
  syl=NULL;
  int total_phrases=0;
  int total_words=0;
  int total_syls=0;
  int is_pau=0;
  int count=0;
  int dist=0;
  int seg_pos_in_syl=0;
  int num_segs_in_syl=0;
  int syl_pos_in_word=0;
  int num_syls_in_word=0;
  int syl_pos_in_phrase=0;
  int num_syls_in_phrase=0;
  int num_words_in_phrase=0;
  int word_pos_in_phrase=0;
  int phrase_pos_in_utt=0;
  int is_content=0;
  int is_stressed=0;

  for(phr=relation_head(utt_relation(u,"Phrase"));phr;phr=item_next(phr),total_phrases++)
    {
      num_syls_in_phrase=0;
      for(num_words_in_phrase=0,w=item_daughter(phr);w;w=item_next(w),num_words_in_phrase++,total_words++)
        {
          sw=item_as(w,"SylStructure");
          for(num_syls_in_word=0,ssyl=item_daughter(sw);
              ssyl;
              ssyl=item_next(ssyl),num_syls_in_word++,num_syls_in_phrase++,total_syls++)
            {
              for(num_segs_in_syl=0,ss=item_daughter(ssyl);ss;ss=item_next(ss),num_segs_in_syl++)
                {
                  item_set_int(ss,"pos_in_syl",num_segs_in_syl);
                }
              item_set_int(ssyl,"num_phones",num_segs_in_syl);
              item_set_int(ssyl,"pos_in_word",num_syls_in_word);
              item_set_int(ssyl,"pos_in_phrase",num_syls_in_phrase);
              item_set_int(ssyl,"pos_in_utt",total_syls);
            }
          item_set_int(w,"num_syls",num_syls_in_word);
          item_set_int(w,"pos_in_phrase",num_words_in_phrase);
          item_set_int(w,"pos_in_utt",total_words);
        }
      item_set_int(phr,"num_syls",num_syls_in_phrase);
      item_set_int(phr,"pos_in_utt",total_phrases);
      item_set_int(phr,"num_words",num_words_in_phrase);
      for(count=0,dist=0,w=item_daughter(phr);w;w=item_next(w))
        {
          item_set_int(w,"num_content_words_before",count);
          item_set_int(w,"dist_to_prev_content_word",dist);
          is_content=cst_streq(ffeature_string(w,"gpos"),"content");
          if(is_content)
            {
              count++;
              dist=1;
            }
          else if(dist>0)
            dist++;
        }
      for(count=0,dist=0,w=item_last_daughter(phr);w;w=item_prev(w))
        {
          item_set_int(w,"num_content_words_after",count);
          item_set_int(w,"dist_to_next_content_word",dist);
          is_content=cst_streq(ffeature_string(w,"gpos"),"content");
          if(is_content)
            {
              count++;
              dist=1;
            }
          else if(dist>0)
            dist++;
        }
      tmp=path_to_item(phr,"daughtern.R:SylStructure.daughtern.R:Syllable");
      syl=path_to_item(phr,"daughter.R:SylStructure.daughter.R:Syllable");
      for(count=(cst_streq(item_feat_string(syl,"stress"),"1")?-1:0),dist=0;syl;syl=item_next(syl))
        {
          item_set_int(syl,"num_stressed_syls_before",(count<0)?0:count);
          item_set_int(syl,"dist_to_prev_stress",dist);
          is_stressed=!cst_streq(item_feat_string(syl,"stress"),"0");
          if(is_stressed)
            {
              count++;
              dist=1;
            }
          else if(dist > 0)
            dist++;
          if(item_equal(syl,tmp))
            break;
        }
      tmp=path_to_item(phr,"daughter.R:SylStructure.daughter.R:Syllable");
      for(count=0,dist=0,syl=path_to_item(phr,"daughtern.R:SylStructure.daughtern.R:Syllable");
          syl;
          syl=item_prev(syl))
        {
          item_set_int(syl,"num_stressed_syls_after",count);
          item_set_int(syl,"dist_to_next_stress",dist);
          is_stressed=!cst_streq(item_feat_string(syl,"stress"),"0");
          if(is_stressed)
            {
              count++;
              dist=1;
            }
          else if(dist>0)
            dist++;
          if(item_equal(syl,tmp))
            break;
        }
    }

  for(s=relation_head(utt_relation(u,"Segment"));s;s=item_next(s))
    {
      is_pau=cst_streq(item_name(s),"pau");
      if(!is_pau)
        {
          ss=item_as(s,"SylStructure");
          ssyl=item_parent(ss);
          syl=item_as(ssyl,"Syllable");
          sw=item_parent(ssyl);
          phr=item_parent(item_as(sw,"Phrase"));
        }
      ns=item_next(s);
      ps=item_prev(s);
      nns=ns?item_next(ns):NULL;
      pps=ps?item_prev(ps):NULL;
      ptr+=sprintf(ptr,"%s",(pps?item_name(pps):"x"));
      ptr+=sprintf(ptr,"^%s",ps?item_name(ps):"x");
      ptr+=sprintf(ptr,"-%s",item_name(s));
      ptr+=sprintf(ptr,"+%s",ns?item_name(ns):"x");
      ptr+=sprintf(ptr,"=%s",nns?item_name(nns):"x");
      if(is_pau)
        ptr+=sprintf(ptr,"@x_x");
      else
        {
          seg_pos_in_syl=item_feat_int(s,"pos_in_syl");
          num_segs_in_syl=item_feat_int(syl,"num_phones");
          ptr+=sprintf(ptr,"@%d_%d",seg_pos_in_syl+1,num_segs_in_syl-seg_pos_in_syl);
        }
      ptr+=sprintf(ptr,"/A:%s",ffeature_string(s,is_pau?"p.R:SylStructure.parent.R:Syllable.stress":"R:SylStructure.parent.R:Syllable.p.stress"));
      ptr+=sprintf(ptr,"_0");
      tmp=path_to_item(s,is_pau?"p.R:SylStructure.parent.R:Syllable":"R:SylStructure.parent.R:Syllable.p");
      ptr+=sprintf(ptr,"_%d",tmp?item_feat_int(tmp,"num_phones"):0);
      ptr+=sprintf(ptr,"/B:%s",is_pau?"x":ffeature_string(s,"R:SylStructure.parent.R:Syllable.stress"));
      ptr+=sprintf(ptr,"-%s",is_pau?"x":"0");
      ptr+=(is_pau?sprintf(ptr,"-x"):sprintf(ptr,"-%d",num_segs_in_syl));
      if(is_pau)
        ptr+=sprintf(ptr,"@x-x");
      else
        {
          syl_pos_in_word=item_feat_int(syl,"pos_in_word");
          num_syls_in_word=item_feat_int(sw,"num_syls");
          ptr+=sprintf(ptr,"@%d-%d",syl_pos_in_word+1,num_syls_in_word-syl_pos_in_word);
        }
      if(is_pau)
        ptr+=sprintf(ptr,"&x-x");
      else
        {
          syl_pos_in_phrase=item_feat_int(syl,"pos_in_phrase");
          num_syls_in_phrase=item_feat_int(phr,"num_syls");
          ptr+=sprintf(ptr,"&%d-%d",syl_pos_in_phrase+1,num_syls_in_phrase-syl_pos_in_phrase);
        }
      if(is_pau)
        ptr+=sprintf(ptr,"#x-x");
      else
        {
          ptr+=sprintf(ptr,"#%d-%d",item_feat_int(syl,"num_stressed_syls_before")+1,item_feat_int(syl,"num_stressed_syls_after")+1);
        }
      ptr+=sprintf(ptr,is_pau?"$x-x":"$1-1");
      if(is_pau)
        ptr+=sprintf(ptr,"!x-x");
      else
        ptr+=sprintf(ptr,"!%d-%d",item_feat_int(syl,"dist_to_prev_stress"),item_feat_int(syl,"dist_to_next_stress"));
      ptr+=sprintf(ptr,is_pau?";x-x":";0-0");
      if(is_pau)
        ptr+=sprintf(ptr,"|x");
      else
        {
          tmp=path_to_item(syl,"R:SylVowel.daughter");
          ptr+=sprintf(ptr,"|%s",tmp?item_name(tmp):"novowel");
        }
      ptr+=sprintf(ptr,"/C:%s",ffeature_string(s,is_pau?"n.R:SylStructure.parent.R:Syllable.stress":"R:SylStructure.parent.R:Syllable.n.stress"));
      ptr+=sprintf(ptr,"+0");
      tmp=path_to_item(s,is_pau?"n.R:SylStructure.parent.R:Syllable":"R:SylStructure.parent.R:Syllable.n");
      ptr+=sprintf(ptr,"+%d",tmp?item_feat_int(tmp,"num_phones"):0);
      ptr+=sprintf(ptr,"/D:%s",ffeature_string(s,is_pau?"p.R:SylStructure.parent.parent.R:Word.gpos":"R:SylStructure.parent.parent.R:Word.p.gpos"));
      tmp=path_to_item(s,is_pau?"p.R:SylStructure.parent.parent.R:Word":"R:SylStructure.parent.parent.R:Word.p");
      ptr+=sprintf(ptr,"_%d",tmp?item_feat_int(tmp,"num_syls"):0);
      ptr+=sprintf(ptr,"/E:%s",is_pau?"x":ffeature_string(sw,"gpos"));
      if(is_pau)
        ptr+=sprintf(ptr,"+x");
      else
        ptr+=sprintf(ptr,"+%d",num_syls_in_word);
      if(is_pau)
        ptr+=sprintf(ptr,"@x+x");
      else
        {
          word_pos_in_phrase=item_feat_int(sw,"pos_in_phrase");
          num_words_in_phrase=item_feat_int(phr,"num_words");
          ptr+=sprintf(ptr,"@%d+%d",word_pos_in_phrase+1,num_words_in_phrase-word_pos_in_phrase);
        }
      if(is_pau)
        ptr+=sprintf(ptr,"&x+x");
      else
        ptr+=sprintf(ptr,"&%d+%d",item_feat_int(sw,"num_content_words_before")+1,item_feat_int(sw,"num_content_words_after"));
      if(is_pau)
        ptr+=sprintf(ptr,"#x+x");
      else
        ptr+=sprintf(ptr,"#%d+%d",item_feat_int(sw,"dist_to_prev_content_word"),item_feat_int(sw,"dist_to_next_content_word"));
      ptr+=sprintf(ptr,"/F:%s",ffeature_string(s,is_pau?"n.R:SylStructure.parent.parent.R:Word.gpos":"R:SylStructure.parent.parent.R:Word.n.gpos"));
      tmp=path_to_item(s,is_pau?"n.R:SylStructure.parent.parent.R:Word":"R:SylStructure.parent.parent.R:Word.n");
      ptr+=sprintf(ptr,"_%d",tmp?item_feat_int(tmp,"num_syls"):0);
      tmp=is_pau?path_to_item(s,"p.R:SylStructure.parent.parent.R:Phrase.parent"):item_prev(phr);
      ptr+=sprintf(ptr,"/G:%d",tmp?item_feat_int(tmp,"num_syls"):0);
      ptr+=sprintf(ptr,"_%d",tmp?item_feat_int(tmp,"num_words"):0);
      if(is_pau)
        ptr+=sprintf(ptr,"/H:x=x^1=%d",total_phrases);
      else
        {
          phrase_pos_in_utt=item_feat_int(phr,"pos_in_utt");
          ptr+=sprintf(ptr,"/H:%d=%d^%d=%d",item_feat_int(phr,"num_syls"),item_feat_int(phr,"num_words"),phrase_pos_in_utt+1,total_phrases-phrase_pos_in_utt);
        }
      ptr+=sprintf(ptr,is_pau?"|0":"|NONE");
      tmp=is_pau?path_to_item(s,"n.R:SylStructure.parent.parent.R:Phrase.parent"):item_next(phr);
      ptr+=sprintf(ptr,"/I:%d",tmp?item_feat_int(tmp,"num_syls"):0);
      ptr+=sprintf(ptr,"=%d",tmp?item_feat_int(tmp,"num_words"):0);
      ptr+=sprintf(ptr,"/J:%d+%d-%d",total_syls,total_words,total_phrases);
      ptr=label;
      item_set_string(s,"hts_label",label);
    }
  return u;
}
