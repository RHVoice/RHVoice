/* Copyright (C) 2011  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include "lib.h"

typedef struct {
  ucs4_t code;
  unsigned int flags;
  const char *name;
} character_info;

static const character_info data[]={
  {0,cs_sp,"пустой\0символ\0"},
  {9,cs_sp,"табуляция\0"},
  {10,cs_nl,"перевод\0строк+и\0"},
  {11,cs_sp,"вертикальная\0табуляция\0"},
  {12,cs_sp,"смена\0страницы\0"},
  {13,cs_nl,"возврат\0каретки\0"},
  {32,cs_sp,"пробел\0"},
  {'!',cs_po|cs_pf,"восклицательный\0знак\0"},
  {'"',cs_pq|cs_pi|cs_pf,"кавычка\0"},
  {'#',cs_s,"решётка\0"},
  {'$',cs_s,"доллар\0"},
  {'%',cs_s,"процент\0"},
  {'&',cs_s,"амперсанд\0"},
  {'\'',cs_pq|cs_pi|cs_pf|cs_en,"апостроф\0"},
  {'(',cs_po|cs_pi,"левая\0круглая\0"},
  {')',cs_po|cs_pf,"правая\0круглая\0"},
  {'*',cs_s,"звёздочка\0"},
  {'+',cs_s,"плюс\0"},
  {',',cs_po|cs_pf,"запятая\0"},
  {'-',cs_pd|cs_pi|cs_pf|cs_ru,"дефис\0"},
  {'.',cs_po|cs_pf,"точка\0"},
  {'/',cs_s,"косая\0черт+а\0"},
  {'0',cs_d,"ноль\0"},
  {'1',cs_d,"один\0"},
  {'2',cs_d,"два\0"},
  {'3',cs_d,"три\0"},
  {'4',cs_d,"четыре\0"},
  {'5',cs_d,"пять\0"},
  {'6',cs_d,"шесть\0"},
  {'7',cs_d,"семь\0"},
  {'8',cs_d,"восемь\0"},
  {'9',cs_d,"девять\0"},
  {':',cs_po|cs_pf,"двоеточие\0"},
  {';',cs_po|cs_pf,"точка\0с\0запятой\0"},
  {'<',cs_s,"меньше\0"},
  {'=',cs_s,"равно\0"},
  {'>',cs_s,"больше\0"},
  {'?',cs_po|cs_pf,"вопросительный\0знак\0"},
  {'@',cs_s,"собачка\0"},
  {'A',cs_lu|cs_lv|cs_en|cs_lw,"a\0"},
  {'B',cs_lu|cs_lc|cs_en,"b\0"},
  {'C',cs_lu|cs_lc|cs_en,"c\0"},
  {'D',cs_lu|cs_lc|cs_en,"d\0"},
  {'E',cs_lu|cs_lv|cs_en,"e\0"},
  {'F',cs_lu|cs_lc|cs_en,"f\0"},
  {'G',cs_lu|cs_lc|cs_en,"g\0"},
  {'H',cs_lu|cs_lc|cs_en,"h\0"},
  {'I',cs_lu|cs_lv|cs_en|cs_lw,"i\0"},
  {'J',cs_lu|cs_lc|cs_en,"j\0"},
  {'K',cs_lu|cs_lc|cs_en,"k\0"},
  {'L',cs_lu|cs_lc|cs_en,"l\0"},
  {'M',cs_lu|cs_lc|cs_en,"m\0"},
  {'N',cs_lu|cs_lc|cs_en,"n\0"},
  {'O',cs_lu|cs_lv|cs_en,"o\0"},
  {'P',cs_lu|cs_lc|cs_en,"p\0"},
  {'Q',cs_lu|cs_lc|cs_en,"q\0"},
  {'R',cs_lu|cs_lc|cs_en,"r\0"},
  {'S',cs_lu|cs_lc|cs_en,"s\0"},
  {'T',cs_lu|cs_lc|cs_en,"t\0"},
  {'U',cs_lu|cs_lv|cs_en,"u\0"},
  {'V',cs_lu|cs_lc|cs_en,"v\0"},
  {'W',cs_lu|cs_lc|cs_en,"w\0"},
  {'X',cs_lu|cs_lc|cs_en,"x\0"},
  {'Y',cs_lu|cs_lc|cs_en,"y\0"},
  {'Z',cs_lu|cs_lc|cs_en,"z\0"},
  {'[',cs_po|cs_pi,"левая\0квадратная\0"},
  {'\\',cs_s,"обратная\0косая\0черт+а\0"},
  {']',cs_po|cs_pf,"правая\0квадратная\0"},
  {'^',cs_s,"крышка\0"},
  {'_',cs_s,"подчёркивание\0"},
  {'`',cs_s,"обратный\0апостроф\0"},
  {'a',cs_ll|cs_lv|cs_en|cs_lw,"a\0"},
  {'b',cs_ll|cs_lc|cs_en,"b\0"},
  {'c',cs_ll|cs_lc|cs_en,"c\0"},
  {'d',cs_ll|cs_lc|cs_en,"d\0"},
  {'e',cs_ll|cs_lv|cs_en,"e\0"},
  {'f',cs_ll|cs_lc|cs_en,"f\0"},
  {'g',cs_ll|cs_lc|cs_en,"g\0"},
  {'h',cs_ll|cs_lc|cs_en,"h\0"},
  {'i',cs_ll|cs_lv|cs_en|cs_lw,"i\0"},
  {'j',cs_ll|cs_lc|cs_en,"j\0"},
  {'k',cs_ll|cs_lc|cs_en,"k\0"},
  {'l',cs_ll|cs_lc|cs_en,"l\0"},
  {'m',cs_ll|cs_lc|cs_en,"m\0"},
  {'n',cs_ll|cs_lc|cs_en,"n\0"},
  {'o',cs_ll|cs_lv|cs_en,"o\0"},
  {'p',cs_ll|cs_lc|cs_en,"p\0"},
  {'q',cs_ll|cs_lc|cs_en,"q\0"},
  {'r',cs_ll|cs_lc|cs_en,"r\0"},
  {'s',cs_ll|cs_lc|cs_en,"s\0"},
  {'t',cs_ll|cs_lc|cs_en,"t\0"},
  {'u',cs_ll|cs_lv|cs_en,"u\0"},
  {'v',cs_ll|cs_lc|cs_en,"v\0"},
  {'w',cs_ll|cs_lc|cs_en,"w\0"},
  {'x',cs_ll|cs_lc|cs_en,"x\0"},
  {'y',cs_ll|cs_lc|cs_en,"y\0"},
  {'z',cs_ll|cs_lc|cs_en,"z\0"},
  {'{',cs_po|cs_pi,"левая\0фигурная\0"},
  {'|',cs_s,"вертикальная\0черт+а\0"},
  {'}',cs_po|cs_pf,"правая\0фигурная\0"},
  {'~',cs_s,"тильда\0"},
  {133,cs_nl,"следующая\0строка\0"},
  {160,cs_sp,"неразрывный\0пробел\0"},
  {163,cs_s,"фунт\0"},
  {1025,cs_lu|cs_lv|cs_ru,"ё\0"},
  {1040,cs_lu|cs_lv|cs_ru|cs_lw,"а\0"},
  {1041,cs_lu|cs_lc|cs_ru,"б\0"},
  {1042,cs_lu|cs_lc|cs_ru|cs_lw,"в\0"},
  {1043,cs_lu|cs_lc|cs_ru,"г\0"},
  {1044,cs_lu|cs_lc|cs_ru,"д\0"},
  {1045,cs_lu|cs_lv|cs_ru,"е\0"},
  {1046,cs_lu|cs_lc|cs_ru|cs_lw,"ж\0"},
  {1047,cs_lu|cs_lc|cs_ru,"з\0"},
  {1048,cs_lu|cs_lv|cs_ru|cs_lw,"и\0"},
  {1049,cs_lu|cs_lc|cs_ru,"й\0"},
  {1050,cs_lu|cs_lc|cs_ru|cs_lw,"к\0"},
  {1051,cs_lu|cs_lc|cs_ru,"л\0"},
  {1052,cs_lu|cs_lc|cs_ru,"м\0"},
  {1053,cs_lu|cs_lc|cs_ru,"н\0"},
  {1054,cs_lu|cs_lv|cs_ru|cs_lw,"о\0"},
  {1055,cs_lu|cs_lc|cs_ru,"п\0"},
  {1056,cs_lu|cs_lc|cs_ru,"р\0"},
  {1057,cs_lu|cs_lc|cs_ru|cs_lw,"с\0"},
  {1058,cs_lu|cs_lc|cs_ru,"т\0"},
  {1059,cs_lu|cs_lv|cs_ru|cs_lw,"у\0"},
  {1060,cs_lu|cs_lc|cs_ru,"ф\0"},
  {1061,cs_lu|cs_lc|cs_ru,"х\0"},
  {1062,cs_lu|cs_lc|cs_ru,"ц\0"},
  {1063,cs_lu|cs_lc|cs_ru,"ч\0"},
  {1064,cs_lu|cs_lc|cs_ru,"ш\0"},
  {1065,cs_lu|cs_lc|cs_ru,"щ\0"},
  {1066,cs_lu|cs_ru,"ъ\0"},
  {1067,cs_lu|cs_lv|cs_ru,"ы\0"},
  {1068,cs_lu|cs_ru,"ь\0"},
  {1069,cs_lu|cs_lv|cs_ru,"э\0"},
  {1070,cs_lu|cs_lv|cs_ru,"ю\0"},
  {1071,cs_lu|cs_lv|cs_ru|cs_lw,"я\0"},
  {1072,cs_ll|cs_lv|cs_ru|cs_lw,"а\0"},
  {1073,cs_ll|cs_lc|cs_ru,"б\0"},
  {1074,cs_ll|cs_lc|cs_ru|cs_lw,"в\0"},
  {1075,cs_ll|cs_lc|cs_ru,"г\0"},
  {1076,cs_ll|cs_lc|cs_ru,"д\0"},
  {1077,cs_ll|cs_lv|cs_ru,"е\0"},
  {1078,cs_ll|cs_lc|cs_ru|cs_lw,"ж\0"},
  {1079,cs_ll|cs_lc|cs_ru,"з\0"},
  {1080,cs_ll|cs_lv|cs_ru|cs_lw,"и\0"},
  {1081,cs_ll|cs_lc|cs_ru,"й\0"},
  {1082,cs_ll|cs_lc|cs_ru|cs_lw,"к\0"},
  {1083,cs_ll|cs_lc|cs_ru,"л\0"},
  {1084,cs_ll|cs_lc|cs_ru,"м\0"},
  {1085,cs_ll|cs_lc|cs_ru,"н\0"},
  {1086,cs_ll|cs_lv|cs_ru|cs_lw,"о\0"},
  {1087,cs_ll|cs_lc|cs_ru,"п\0"},
  {1088,cs_ll|cs_lc|cs_ru,"р\0"},
  {1089,cs_ll|cs_lc|cs_ru|cs_lw,"с\0"},
  {1090,cs_ll|cs_lc|cs_ru,"т\0"},
  {1091,cs_ll|cs_lv|cs_ru|cs_lw,"у\0"},
  {1092,cs_ll|cs_lc|cs_ru,"ф\0"},
  {1093,cs_ll|cs_lc|cs_ru,"х\0"},
  {1094,cs_ll|cs_lc|cs_ru,"ц\0"},
  {1095,cs_ll|cs_lc|cs_ru,"ч\0"},
  {1096,cs_ll|cs_lc|cs_ru,"ш\0"},
  {1097,cs_ll|cs_lc|cs_ru,"щ\0"},
  {1098,cs_ll|cs_ru,"ъ\0"},
  {1099,cs_ll|cs_lv|cs_ru,"ы\0"},
  {1100,cs_ll|cs_ru,"ь\0"},
  {1101,cs_ll|cs_lv|cs_ru,"э\0"},
  {1102,cs_ll|cs_lv|cs_ru,"ю\0"},
  {1103,cs_ll|cs_lv|cs_ru|cs_lw,"я\0"},
  {1105,cs_ll|cs_lv|cs_ru,"ё\0"},
  {8232,cs_nl,"разделитель\0строк\0"},
  {8233,cs_pr,"разделитель\0абзацев\0"},
  {8364,cs_s,"евро\0"}
};

static int cmp(const void *key,const void *element)
{
  const ucs4_t *pc=(const ucs4_t*)key;
  const character_info *pci=(const character_info*)element;
  return (*pc-pci->code);
}

unsigned int classify_character(ucs4_t c)
{
  ucs4_t c0=c;
  const character_info *pci=(const character_info*)bsearch(&c0,data,
                                                           sizeof(data)/sizeof(character_info),
                                                           sizeof(character_info),cmp);
  return (pci==NULL)?0:pci->flags;
}

unsigned int classify_characters(const uint32_t *chars,size_t n)
{
  if((chars==NULL)||(n==0)) return 0;
  unsigned int flags=0xffffffff;
  size_t i;
  for(i=0;i<n;i++)
    {
      flags&=classify_character(chars[i]);
      if(flags==0) return 0;
    }
  return flags;
}

const char *character_name(ucs4_t c)
{
  ucs4_t c0=c;
  const character_info *pci=(const character_info*)bsearch(&c0,data,
                                                           sizeof(data)/sizeof(character_info),
                                                           sizeof(character_info),cmp);
  return (pci==NULL)?NULL:pci->name;
}
