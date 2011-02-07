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
} character_info;

static const character_info data[]={
  {0,cs_sp},
  {10,cs_nl},
  {11,cs_sp},
  {12,cs_sp},
  {13,cs_nl},
  {32,cs_sp},
  {'!',cs_po|cs_pf},
  {'"',cs_pq|cs_pi|cs_pf},
  {'\'',cs_pq|cs_pi|cs_pf},
  {'(',cs_po|cs_pi},
  {')',cs_po|cs_pf},
  {',',cs_po|cs_pf},
  {'-',cs_pd|cs_pi|cs_pf},
  {'.',cs_po|cs_pf},
  {'0',cs_d},
  {'1',cs_d},
  {'2',cs_d},
  {'3',cs_d},
  {'4',cs_d},
  {'5',cs_d},
  {'6',cs_d},
  {'7',cs_d},
  {'8',cs_d},
  {'9',cs_d},
  {':',cs_po|cs_pf},
  {';',cs_po|cs_pf},
  {'?',cs_po|cs_pf},
  {'A',cs_lu|cs_lv|cs_en},
  {'B',cs_lu|cs_lc|cs_en},
  {'C',cs_lu|cs_lc|cs_en},
  {'D',cs_lu|cs_lc|cs_en},
  {'E',cs_lu|cs_lv|cs_en},
  {'F',cs_lu|cs_lc|cs_en},
  {'G',cs_lu|cs_lc|cs_en},
  {'H',cs_lu|cs_lc|cs_en},
  {'I',cs_lu|cs_lv|cs_en},
  {'J',cs_lu|cs_lc|cs_en},
  {'K',cs_lu|cs_lc|cs_en},
  {'L',cs_lu|cs_lc|cs_en},
  {'M',cs_lu|cs_lc|cs_en},
  {'N',cs_lu|cs_lc|cs_en},
  {'O',cs_lu|cs_lv|cs_en},
  {'P',cs_lu|cs_lc|cs_en},
  {'Q',cs_lu|cs_lc|cs_en},
  {'R',cs_lu|cs_lc|cs_en},
  {'S',cs_lu|cs_lc|cs_en},
  {'T',cs_lu|cs_lc|cs_en},
  {'U',cs_lu|cs_lv|cs_en},
  {'V',cs_lu|cs_lc|cs_en},
  {'W',cs_lu|cs_lc|cs_en},
  {'X',cs_lu|cs_lc|cs_en},
  {'Y',cs_lu|cs_lc|cs_en},
  {'Z',cs_lu|cs_lc|cs_en},
  {'a',cs_ll|cs_lv|cs_en},
  {'b',cs_ll|cs_lc|cs_en},
  {'c',cs_ll|cs_lc|cs_en},
  {'d',cs_ll|cs_lc|cs_en},
  {'e',cs_ll|cs_lv|cs_en},
  {'f',cs_ll|cs_lc|cs_en},
  {'g',cs_ll|cs_lc|cs_en},
  {'h',cs_ll|cs_lc|cs_en},
  {'i',cs_ll|cs_lv|cs_en},
  {'j',cs_ll|cs_lc|cs_en},
  {'k',cs_ll|cs_lc|cs_en},
  {'l',cs_ll|cs_lc|cs_en},
  {'m',cs_ll|cs_lc|cs_en},
  {'n',cs_ll|cs_lc|cs_en},
  {'o',cs_ll|cs_lv|cs_en},
  {'p',cs_ll|cs_lc|cs_en},
  {'q',cs_ll|cs_lc|cs_en},
  {'r',cs_ll|cs_lc|cs_en},
  {'s',cs_ll|cs_lc|cs_en},
  {'t',cs_ll|cs_lc|cs_en},
  {'u',cs_ll|cs_lv|cs_en},
  {'v',cs_ll|cs_lc|cs_en},
  {'w',cs_ll|cs_lc|cs_en},
  {'x',cs_ll|cs_lc|cs_en},
  {'y',cs_ll|cs_lc|cs_en},
  {'z',cs_ll|cs_lc|cs_en},
  {133,cs_nl},
  {160,cs_sp},
  {1025,cs_lu|cs_lv|cs_ru},     /* Ё */
  {1040,cs_lu|cs_lv|cs_ru},     /* А */
  {1041,cs_lu|cs_lc|cs_ru},           /* Б */
  {1042,cs_lu|cs_lc|cs_ru},           /* В */
  {1043,cs_lu|cs_lc|cs_ru},           /* Г */
  {1044,cs_lu|cs_lc|cs_ru},           /* Д */
  {1045,cs_lu|cs_lv|cs_ru},     /* Е */
  {1046,cs_lu|cs_lc|cs_ru},           /* Ж */
  {1047,cs_lu|cs_lc|cs_ru},           /* З */
  {1048,cs_lu|cs_lv|cs_ru},     /* И */
  {1049,cs_lu|cs_lc|cs_ru},           /* Й */
  {1050,cs_lu|cs_lc|cs_ru},           /* К */
  {1051,cs_lu|cs_lc|cs_ru},           /* Л */
  {1052,cs_lu|cs_lc|cs_ru},           /* М */
  {1053,cs_lu|cs_lc|cs_ru},           /* Н */
  {1054,cs_lu|cs_lv|cs_ru},           /* О */
  {1055,cs_lu|cs_lc|cs_ru},           /* П */
  {1056,cs_lu|cs_lc|cs_ru},           /* Р */
  {1057,cs_lu|cs_lc|cs_ru},           /* С */
  {1058,cs_lu|cs_lc|cs_ru},           /* Т */
  {1059,cs_lu|cs_lv|cs_ru},           /* У */
  {1060,cs_lu|cs_lc|cs_ru},           /* Ф */
  {1061,cs_lu|cs_lc|cs_ru},           /* Х */
  {1062,cs_lu|cs_lc|cs_ru},           /* Ц */
  {1063,cs_lu|cs_lc|cs_ru},           /* Ч */
  {1064,cs_lu|cs_lc|cs_ru},           /* Ш */
  {1065,cs_lu|cs_lc|cs_ru},           /* Щ */
  {1066,cs_lu|cs_ru},           /* Ъ */
  {1067,cs_lu|cs_lv|cs_ru},           /* Ы */
  {1068,cs_lu|cs_ru},           /* Ь */
  {1069,cs_lu|cs_lv|cs_ru},           /* Э */
  {1070,cs_lu|cs_lv|cs_ru},           /* Ю */
  {1071,cs_lu|cs_lv|cs_ru},           /* Я */
  {1072,cs_ll|cs_lv|cs_ru},           /* а */
  {1073,cs_ll|cs_lc|cs_ru},           /* б */
  {1074,cs_ll|cs_lc|cs_ru},           /* в */
  {1075,cs_ll|cs_lc|cs_ru},           /* г */
  {1076,cs_ll|cs_lc|cs_ru},           /* д */
  {1077,cs_ll|cs_lv|cs_ru},           /* е */
  {1078,cs_ll|cs_lc|cs_ru},           /* ж */
  {1079,cs_ll|cs_lc|cs_ru},           /* з */
  {1080,cs_ll|cs_lv|cs_ru},           /* и */
  {1081,cs_ll|cs_lc|cs_ru},           /* й */
  {1082,cs_ll|cs_lc|cs_ru},           /* к */
  {1083,cs_ll|cs_lc|cs_ru},           /* л */
  {1084,cs_ll|cs_lc|cs_ru},           /* м */
  {1085,cs_ll|cs_lc|cs_ru},           /* н */
  {1086,cs_ll|cs_lv|cs_ru},           /* о */
  {1087,cs_ll|cs_lc|cs_ru},           /* п */
  {1088,cs_ll|cs_lc|cs_ru},           /* р */
  {1089,cs_ll|cs_lc|cs_ru},           /* с */
  {1090,cs_ll|cs_lc|cs_ru},           /* т */
  {1091,cs_ll|cs_lv|cs_ru},           /* у */
  {1092,cs_ll|cs_lc|cs_ru},           /* ф */
  {1093,cs_ll|cs_lc|cs_ru},           /* х */
  {1094,cs_ll|cs_lc|cs_ru},           /* ц */
  {1095,cs_ll|cs_lc|cs_ru},           /* ч */
  {106,cs_ll|cs_lc|cs_ru},           /* ш */
  {1097,cs_ll|cs_lc|cs_ru},           /* щ */
  {1098,cs_ll|cs_ru},           /* ъ */
  {1099,cs_ll|cs_lv|cs_ru},           /* ы */
  {1100,cs_ll|cs_ru},           /* ь */
  {1101,cs_ll|cs_lv|cs_ru},     /* э */
  {1102,cs_ll|cs_lv|cs_ru},     /* ю */
  {1103,cs_ll|cs_lv|cs_ru},     /* я */
  {1105,cs_ll|cs_lv|cs_ru},     /* ё */
  {8232,cs_nl},
  {8233,cs_pr}
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

