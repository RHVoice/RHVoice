#include <cst_regex.h>
static const unsigned char ru_punc_minor_break_rxprog[] = {
   156, 6, 0, 37, 1, 0, 3, 10, 0, 6, 3, 0, 0, 4, 0, 8, 
   45, 44, 58, 59, 0, 6, 0, 8, 8, 0, 8, 34, 0, 6, 0, 3, 
   9, 0, 3, 2, 0, 3, 0, 0, 0, 
};
static const cst_regex ru_punc_minor_break_rx = {
   0, 1, NULL, 0, 41,
   (char *)ru_punc_minor_break_rxprog
};
const cst_regex * const ru_punc_minor_break = &ru_punc_minor_break_rx;

static const unsigned char ru_punc_major_break_rxprog[] = {
   156, 6, 0, 36, 1, 0, 3, 10, 0, 6, 3, 0, 0, 4, 0, 7, 
   46, 63, 33, 0, 6, 0, 8, 8, 0, 8, 34, 0, 6, 0, 3, 9, 
   0, 3, 2, 0, 3, 0, 0, 0, 
};
static const cst_regex ru_punc_major_break_rx = {
   0, 1, NULL, 0, 40,
   (char *)ru_punc_major_break_rxprog
};
const cst_regex * const ru_punc_major_break = &ru_punc_major_break_rx;

