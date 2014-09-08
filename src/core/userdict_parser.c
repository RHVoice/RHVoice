/* Driver template for the LEMON parser generator.
** The author disclaims copyright to this source code.
*/
/* First off, code is included that follows the "include" declaration
** in the input grammar file. */
#include <stdio.h>
#line 16 "userdict_parser.g"
#include "core/userdict.hpp"
#line 10 "userdict_parser.c"
/* Next is all token values, in a form suitable for use by makeheaders.
** This section will be null unless lemon is run with the -m switch.
*/
/* 
** These constants (all generated automatically by the parser generator)
** specify the various kinds of tokens (terminals) that the parser
** understands. 
**
** Each symbol here is a terminal symbol in the grammar.
*/
/* Make sure the INTERFACE macro is defined.
*/
#ifndef INTERFACE
# define INTERFACE 1
#endif
/* The next thing included is series of defines which control
** various aspects of the generated parser.
**    YYCODETYPE         is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 terminals
**                       and nonterminals.  "int" is used otherwise.
**    YYNOCODE           is a number of type YYCODETYPE which corresponds
**                       to no legal terminal or nonterminal number.  This
**                       number is used to fill in empty slots of the hash 
**                       table.
**    YYFALLBACK         If defined, this indicates that one or more tokens
**                       have fall-back values which should be used if the
**                       original value of the token will not parse.
**    YYACTIONTYPE       is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 rules and
**                       states combined.  "int" is used otherwise.
**    userdictParseTOKENTYPE     is the data type used for minor tokens given 
**                       directly to the parser from the tokenizer.
**    YYMINORTYPE        is the data type used for all minor tokens.
**                       This is typically a union of many types, one of
**                       which is userdictParseTOKENTYPE.  The entry in the union
**                       for base tokens is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
**                       zero the stack is dynamically sized using realloc()
**    userdictParseARG_SDECL     A static variable declaration for the %extra_argument
**    userdictParseARG_PDECL     A parameter declaration for the %extra_argument
**    userdictParseARG_STORE     Code to store %extra_argument into yypParser
**    userdictParseARG_FETCH     Code to extract %extra_argument from yypParser
**    YYNSTATE           the combined number of states.
**    YYNRULE            the number of rules in the grammar
**    YYERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
*/
#define YYCODETYPE unsigned char
#define YYNOCODE 57
#define YYACTIONTYPE unsigned char
#define userdictParseTOKENTYPE RHVoice::userdict::token*
typedef union {
  int yyinit;
  userdictParseTOKENTYPE yy0;
  RHVoice::userdict::ruleset* yy98;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define userdictParseARG_SDECL RHVoice::userdict::parse_state* ps;
#define userdictParseARG_PDECL ,RHVoice::userdict::parse_state* ps
#define userdictParseARG_FETCH RHVoice::userdict::parse_state* ps = yypParser->ps
#define userdictParseARG_STORE yypParser->ps = ps
#define YYNSTATE 86
#define YYNRULE 65
#define YY_NO_ACTION      (YYNSTATE+YYNRULE+2)
#define YY_ACCEPT_ACTION  (YYNSTATE+YYNRULE+1)
#define YY_ERROR_ACTION   (YYNSTATE+YYNRULE)

/* The yyzerominor constant is used to initialize instances of
** YYMINORTYPE objects to zero. */
static const YYMINORTYPE yyzerominor = { 0 };

/* Define the yytestcase() macro to be a no-op if is not already defined
** otherwise.
**
** Applications can choose to define yytestcase() in the %include section
** to a macro that can assist in verifying code coverage.  For production
** code the yytestcase() macro should be turned off.  But it is useful
** for testing.
*/
#ifndef yytestcase
# define yytestcase(X)
#endif


/* Next are the tables used to determine what action to take based on the
** current state and lookahead token.  These tables are used to implement
** functions that take a state number and lookahead value and return an
** action integer.  
**
** Suppose the action integer is N.  Then the action is determined as
** follows
**
**   0 <= N < YYNSTATE                  Shift N.  That is, push the lookahead
**                                      token onto the stack and goto state N.
**
**   YYNSTATE <= N < YYNSTATE+YYNRULE   Reduce by rule N-YYNSTATE.
**
**   N == YYNSTATE+YYNRULE              A syntax error has occurred.
**
**   N == YYNSTATE+YYNRULE+1            The parser accepts its input.
**
**   N == YYNSTATE+YYNRULE+2            No such action.  Denotes unused
**                                      slots in the yy_action[] table.
**
** The action table is constructed as a single large table named yy_action[].
** Given state S and lookahead X, the action is computed as
**
**      yy_action[ yy_shift_ofst[S] + X ]
**
** If the index value yy_shift_ofst[S]+X is out of range or if the value
** yy_lookahead[yy_shift_ofst[S]+X] is not equal to X or if yy_shift_ofst[S]
** is equal to YY_SHIFT_USE_DFLT, it means that the action is not in the table
** and that yy_default[S] should be used instead.  
**
** The formula above is for computing the action when the lookahead is
** a terminal symbol.  If the lookahead is a non-terminal (as occurs after
** a reduce action) then the yy_reduce_ofst[] array is used in place of
** the yy_shift_ofst[] array and YY_REDUCE_USE_DFLT is used in place of
** YY_SHIFT_USE_DFLT.
**
** The following are the tables generated in this section:
**
**  yy_action[]        A single table containing all actions.
**  yy_lookahead[]     A table containing the lookahead for each entry in
**                     yy_action.  Used to detect hash collisions.
**  yy_shift_ofst[]    For each state, the offset into yy_action for
**                     shifting terminals.
**  yy_reduce_ofst[]   For each state, the offset into yy_action for
**                     shifting non-terminals after a reduce.
**  yy_default[]       Default action for each state.
*/
#define YY_ACTTAB_COUNT (181)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */    18,   62,   11,   43,   33,   45,    2,   50,   78,   82,
 /*    10 */    33,   45,    2,   50,   78,   35,    3,   44,   58,   79,
 /*    20 */    78,   80,   77,   59,   58,   48,   31,   76,   77,   59,
 /*    30 */    58,   86,   31,   76,   77,   59,   53,   21,   47,   76,
 /*    40 */    53,   22,   64,   63,   25,   42,   26,   24,   12,   16,
 /*    50 */    69,   81,    2,   50,   78,   68,   67,   71,   87,   73,
 /*    60 */    70,   72,   15,   69,   58,   13,   58,   74,   77,   59,
 /*    70 */    60,   59,   73,   76,   72,   17,   73,    7,   72,    5,
 /*    80 */    57,   56,    6,   70,   65,   52,   51,   30,   65,   52,
 /*    90 */    46,   12,   16,   69,   10,   49,   12,   16,   69,   75,
 /*   100 */   140,  141,   73,   71,   72,    9,   70,   73,    8,   72,
 /*   110 */    19,   41,   12,   16,   69,   66,   40,   12,   16,   69,
 /*   120 */    39,   65,   27,   73,   83,   72,    4,   32,   73,  153,
 /*   130 */    72,   58,   23,   54,   14,   55,   59,   36,   85,   84,
 /*   140 */   153,    1,  153,   58,  153,   61,   14,   55,   59,  153,
 /*   150 */    17,  153,    7,  153,  153,   57,   56,   17,   70,    7,
 /*   160 */   153,  153,   29,   28,   20,   70,    7,  153,   34,   57,
 /*   170 */    56,   38,  152,  153,  153,  153,  153,  153,  153,  153,
 /*   180 */    37,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */    22,    9,   10,   25,   26,   27,   28,   29,   30,   25,
 /*    10 */    26,   27,   28,   29,   30,    1,    3,    4,   40,   29,
 /*    20 */    30,   31,   44,   45,   40,   39,   48,   49,   44,   45,
 /*    30 */    40,    0,   48,   49,   44,   45,   50,   51,   52,   49,
 /*    40 */    50,   51,   52,   53,   32,   33,   34,   35,   36,   37,
 /*    50 */    38,   27,   28,   29,   30,   11,   12,   11,    0,   47,
 /*    60 */    14,   49,   37,   38,   40,    2,   40,   38,   44,   45,
 /*    70 */    44,   45,   47,   49,   49,    6,   47,    8,   49,    5,
 /*    80 */    11,   12,    2,   14,   15,   16,   17,   32,   15,   16,
 /*    90 */    35,   36,   37,   38,    5,   35,   36,   37,   38,    7,
 /*   100 */     2,    2,   47,   11,   49,    3,   14,   47,    3,   49,
 /*   110 */     2,   35,   36,   37,   38,   50,   35,   36,   37,   38,
 /*   120 */     3,   15,   55,   47,   24,   49,   55,   48,   47,   56,
 /*   130 */    49,   40,   41,   42,   43,   44,   45,   19,   20,   21,
 /*   140 */    56,   23,   56,   40,   56,   42,   43,   44,   45,   56,
 /*   150 */     6,   56,    8,   56,   56,   11,   12,    6,   14,    8,
 /*   160 */    56,   56,   11,   12,   46,   14,    8,   56,    1,   11,
 /*   170 */    12,    4,   54,   56,   56,   56,   56,   56,   56,   56,
 /*   180 */    13,
};
#define YY_SHIFT_USE_DFLT (-9)
#define YY_SHIFT_COUNT (36)
#define YY_SHIFT_MIN   (-8)
#define YY_SHIFT_MAX   (167)
static const short yy_shift_ofst[] = {
 /*     0 */   167,  151,   69,  151,   46,  144,   46,  158,   46,   46,
 /*    10 */    46,  158,   73,   46,  158,   92,   46,   44,   13,  117,
 /*    20 */   117,  106,  106,   -8,  105,   89,  102,  108,   99,   98,
 /*    30 */    89,   80,   63,   74,   58,   31,   14,
};
#define YY_REDUCE_USE_DFLT (-23)
#define YY_REDUCE_COUNT (22)
#define YY_REDUCE_MIN   (-22)
#define YY_REDUCE_MAX   (118)
static const signed char yy_reduce_ofst[] = {
 /*     0 */   118,  -22,  -10,  -16,   12,   24,   55,   91,   81,   76,
 /*    10 */    60,  103,  -14,   25,   26,   29,   29,   79,  100,   71,
 /*    20 */    67,   65,   65,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */    95,  151,  109,  151,  151,  151,  151,  117,  151,  151,
 /*    10 */   151,  117,  130,  151,  116,  151,  129,  151,   97,  149,
 /*    20 */   149,  131,  110,  151,  125,  123,  122,  151,  138,  137,
 /*    30 */    99,  151,  151,   98,  151,  151,  151,  139,   94,  150,
 /*    40 */   127,  126,   91,   93,   96,  101,  125,  132,  128,  124,
 /*    50 */   104,  148,  147,  145,  115,  119,  138,  137,  121,  120,
 /*    60 */   118,  114,  113,  112,  111,  146,  144,  141,  140,  134,
 /*    70 */   143,  142,  136,  135,  133,  108,  107,  106,  105,  103,
 /*    80 */   102,  100,   92,   90,   89,   88,
};

/* The next table maps tokens into fallback tokens.  If a construct
** like the following:
** 
**      %fallback ID X Y Z.
**
** appears in the grammar, then ID becomes a fallback token for X, Y,
** and Z.  Whenever one of the tokens X, Y, or Z is input to the parser
** but it does not parse, the type of the token is changed to ID and
** the parse is retried before an error is thrown.
*/
#ifdef YYFALLBACK
static const YYCODETYPE yyFallback[] = {
};
#endif /* YYFALLBACK */

/* The following structure represents a single element of the
** parser's stack.  Information stored includes:
**
**   +  The state number for the parser at this level of the stack.
**
**   +  The value of the token stored at this level of the stack.
**      (In other words, the "major" token.)
**
**   +  The semantic value stored at this level of the stack.  This is
**      the information used by the action routines in the grammar.
**      It is sometimes called the "minor" token.
*/
struct yyStackEntry {
  YYACTIONTYPE stateno;  /* The state-number */
  YYCODETYPE major;      /* The major token value.  This is the code
                         ** number for the token at this stack level */
  YYMINORTYPE minor;     /* The user-supplied minor token value.  This
                         ** is the value of the token  */
};
typedef struct yyStackEntry yyStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser {
  int yyidx;                    /* Index of top element in stack */
#ifdef YYTRACKMAXSTACKDEPTH
  int yyidxMax;                 /* Maximum value of yyidx */
#endif
  int yyerrcnt;                 /* Shifts left before out of the error */
  userdictParseARG_SDECL                /* A place to hold %extra_argument */
#if YYSTACKDEPTH<=0
  int yystksz;                  /* Current side of the stack */
  yyStackEntry *yystack;        /* The parser's stack */
#else
  yyStackEntry yystack[YYSTACKDEPTH];  /* The parser's stack */
#endif
};
typedef struct yyParser yyParser;

#ifndef NDEBUG
#include <stdio.h>
static FILE *yyTraceFILE = 0;
static char *yyTracePrompt = 0;
#endif /* NDEBUG */

#ifndef NDEBUG
/* 
** Turn parser tracing on by giving a stream to which to write the trace
** and a prompt to preface each trace message.  Tracing is turned off
** by making either argument NULL 
**
** Inputs:
** <ul>
** <li> A FILE* to which trace output should be written.
**      If NULL, then tracing is turned off.
** <li> A prefix string written at the beginning of every
**      line of trace output.  If NULL, then tracing is
**      turned off.
** </ul>
**
** Outputs:
** None.
*/
void userdictParseTrace(FILE *TraceFILE, char *zTracePrompt){
  yyTraceFILE = TraceFILE;
  yyTracePrompt = zTracePrompt;
  if( yyTraceFILE==0 ) yyTracePrompt = 0;
  else if( yyTracePrompt==0 ) yyTraceFILE = 0;
}
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *const yyTokenName[] = { 
  "$",             "NEWLINE",       "EQUALS",        "SPACE",       
  "STAR",          "AND",           "LPAREN",        "RPAREN",      
  "LBRACE",        "RBRACE",        "COMMA",         "NATIVE_LETTERS",
  "ENGLISH_LETTERS",  "SYM",           "PLUS",          "STRESSED_SYL_NUMBER",
  "UNSTRESSED",    "INITIALISM",    "error",         "rule",        
  "collocation_rule",  "symbol_rule",   "tokens",        "prefix",      
  "suffix",        "token",         "words",         "word",        
  "inline_rewrites",  "inline_rewrite",  "inline_replacement",  "word_flags",  
  "token_pron",    "symbol_pron",   "spaced_symbol_pron",  "word_pron",   
  "pron_repr",     "text_pron_repr",  "text_pron_repr_atom",  "word_pron_flags",
  "set",           "set_elements",  "set_element",   "pattern",     
  "subpattern",    "substring",     "symbol",        "insertion",   
  "deletion",      "stress_mark",   "stressed_syl_number",  "stressed_syl_numbers",
  "unstressed_flag",  "initialism_flag",  "line",          "space",       
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "line ::= rule NEWLINE",
 /*   1 */ "line ::= NEWLINE",
 /*   2 */ "rule ::= collocation_rule",
 /*   3 */ "rule ::= symbol_rule",
 /*   4 */ "collocation_rule ::= prefix tokens suffix",
 /*   5 */ "symbol_rule ::= symbol space EQUALS space symbol_pron",
 /*   6 */ "tokens ::= tokens SPACE token",
 /*   7 */ "tokens ::= token",
 /*   8 */ "prefix ::= STAR",
 /*   9 */ "prefix ::=",
 /*  10 */ "suffix ::= STAR",
 /*  11 */ "suffix ::=",
 /*  12 */ "token ::= words",
 /*  13 */ "token ::= deletion EQUALS token_pron",
 /*  14 */ "words ::= words AND word",
 /*  15 */ "words ::= word",
 /*  16 */ "word ::= inline_rewrites word_flags",
 /*  17 */ "inline_rewrites ::= inline_rewrites inline_rewrite",
 /*  18 */ "inline_rewrites ::= inline_rewrite",
 /*  19 */ "inline_rewrite ::= inline_replacement",
 /*  20 */ "inline_rewrite ::= subpattern",
 /*  21 */ "inline_rewrite ::= stress_mark",
 /*  22 */ "inline_replacement ::= LPAREN deletion EQUALS text_pron_repr RPAREN",
 /*  23 */ "word_flags ::=",
 /*  24 */ "word_flags ::= stressed_syl_numbers",
 /*  25 */ "word_flags ::= unstressed_flag",
 /*  26 */ "word_flags ::= initialism_flag",
 /*  27 */ "set ::= LBRACE set_elements RBRACE",
 /*  28 */ "set_elements ::= set_elements COMMA set_element",
 /*  29 */ "set_elements ::= set_element",
 /*  30 */ "set_element ::= pattern",
 /*  31 */ "set_element ::=",
 /*  32 */ "pattern ::= pattern subpattern",
 /*  33 */ "pattern ::= subpattern",
 /*  34 */ "subpattern ::= substring",
 /*  35 */ "subpattern ::= set",
 /*  36 */ "symbol_pron ::= spaced_symbol_pron",
 /*  37 */ "symbol_pron ::= token_pron",
 /*  38 */ "token_pron ::= token_pron AND word_pron",
 /*  39 */ "token_pron ::= word_pron",
 /*  40 */ "spaced_symbol_pron ::= spaced_symbol_pron SPACE word_pron",
 /*  41 */ "spaced_symbol_pron ::= word_pron SPACE word_pron",
 /*  42 */ "word_pron ::= pron_repr word_pron_flags",
 /*  43 */ "pron_repr ::= text_pron_repr",
 /*  44 */ "word_pron_flags ::=",
 /*  45 */ "word_pron_flags ::= stressed_syl_numbers",
 /*  46 */ "word_pron_flags ::= unstressed_flag",
 /*  47 */ "text_pron_repr ::= text_pron_repr text_pron_repr_atom",
 /*  48 */ "text_pron_repr ::= text_pron_repr_atom",
 /*  49 */ "text_pron_repr_atom ::= insertion",
 /*  50 */ "text_pron_repr_atom ::= stress_mark",
 /*  51 */ "substring ::= NATIVE_LETTERS",
 /*  52 */ "substring ::= ENGLISH_LETTERS",
 /*  53 */ "symbol ::= SYM",
 /*  54 */ "deletion ::= NATIVE_LETTERS",
 /*  55 */ "deletion ::= ENGLISH_LETTERS",
 /*  56 */ "insertion ::= NATIVE_LETTERS",
 /*  57 */ "stress_mark ::= PLUS",
 /*  58 */ "stressed_syl_numbers ::= stressed_syl_numbers stressed_syl_number",
 /*  59 */ "stressed_syl_numbers ::= stressed_syl_number",
 /*  60 */ "stressed_syl_number ::= STRESSED_SYL_NUMBER",
 /*  61 */ "unstressed_flag ::= UNSTRESSED",
 /*  62 */ "initialism_flag ::= INITIALISM",
 /*  63 */ "space ::=",
 /*  64 */ "space ::= SPACE",
};
#endif /* NDEBUG */


#if YYSTACKDEPTH<=0
/*
** Try to increase the size of the parser stack.
*/
static void yyGrowStack(yyParser *p){
  int newSize;
  yyStackEntry *pNew;

  newSize = p->yystksz*2 + 100;
  pNew = realloc(p->yystack, newSize*sizeof(pNew[0]));
  if( pNew ){
    p->yystack = pNew;
    p->yystksz = newSize;
#ifndef NDEBUG
    if( yyTraceFILE ){
      fprintf(yyTraceFILE,"%sStack grows to %d entries!\n",
              yyTracePrompt, p->yystksz);
    }
#endif
  }
}
#endif

/* 
** This function allocates a new parser.
** The only argument is a pointer to a function which works like
** malloc.
**
** Inputs:
** A pointer to the function used to allocate memory.
**
** Outputs:
** A pointer to a parser.  This pointer is used in subsequent calls
** to userdictParse and userdictParseFree.
*/
void *userdictParseAlloc(void *(*mallocProc)(size_t)){
  yyParser *pParser;
  pParser = (yyParser*)(*mallocProc)( (size_t)sizeof(yyParser) );
  if( pParser ){
    pParser->yyidx = -1;
#ifdef YYTRACKMAXSTACKDEPTH
    pParser->yyidxMax = 0;
#endif
#if YYSTACKDEPTH<=0
    pParser->yystack = NULL;
    pParser->yystksz = 0;
    yyGrowStack(pParser);
#endif
  }
  return pParser;
}

/* The following function deletes the value associated with a
** symbol.  The symbol can be either a terminal or nonterminal.
** "yymajor" is the symbol code, and "yypminor" is a pointer to
** the value.
*/
static void yy_destructor(
  yyParser *yypParser,    /* The parser */
  YYCODETYPE yymajor,     /* Type code for object to destroy */
  YYMINORTYPE *yypminor   /* The object to be destroyed */
){
  userdictParseARG_FETCH;
  switch( yymajor ){
    /* Here is inserted the actions which take place when a
    ** terminal or non-terminal is destroyed.  This can happen
    ** when the symbol is popped from the stack during a
    ** reduce or during error processing or when a parser is 
    ** being destroyed before it is finished parsing.
    **
    ** Note: during a reduce, the only symbols destroyed are those
    ** which appear on the RHS of the rule, but which are not used
    ** inside the C code.
    */
      /* TERMINAL Destructor */
    case 1: /* NEWLINE */
    case 2: /* EQUALS */
    case 3: /* SPACE */
    case 4: /* STAR */
    case 5: /* AND */
    case 6: /* LPAREN */
    case 7: /* RPAREN */
    case 8: /* LBRACE */
    case 9: /* RBRACE */
    case 10: /* COMMA */
    case 11: /* NATIVE_LETTERS */
    case 12: /* ENGLISH_LETTERS */
    case 13: /* SYM */
    case 14: /* PLUS */
    case 15: /* STRESSED_SYL_NUMBER */
    case 16: /* UNSTRESSED */
    case 17: /* INITIALISM */
{
#line 24 "userdict_parser.g"
delete (yypminor->yy0);
#line 495 "userdict_parser.c"
}
      break;
    case 19: /* rule */
    case 20: /* collocation_rule */
    case 21: /* symbol_rule */
    case 22: /* tokens */
    case 23: /* prefix */
    case 24: /* suffix */
    case 25: /* token */
    case 26: /* words */
    case 27: /* word */
    case 28: /* inline_rewrites */
    case 29: /* inline_rewrite */
    case 30: /* inline_replacement */
    case 31: /* word_flags */
    case 32: /* token_pron */
    case 33: /* symbol_pron */
    case 34: /* spaced_symbol_pron */
    case 35: /* word_pron */
    case 36: /* pron_repr */
    case 37: /* text_pron_repr */
    case 38: /* text_pron_repr_atom */
    case 39: /* word_pron_flags */
    case 40: /* set */
    case 41: /* set_elements */
    case 42: /* set_element */
    case 43: /* pattern */
    case 44: /* subpattern */
    case 45: /* substring */
    case 46: /* symbol */
    case 47: /* insertion */
    case 48: /* deletion */
    case 49: /* stress_mark */
    case 50: /* stressed_syl_number */
    case 51: /* stressed_syl_numbers */
    case 52: /* unstressed_flag */
    case 53: /* initialism_flag */
{
#line 27 "userdict_parser.g"
delete (yypminor->yy98);
#line 536 "userdict_parser.c"
}
      break;
    default:  break;   /* If no destructor action specified: do nothing */
  }
}

/*
** Pop the parser's stack once.
**
** If there is a destructor routine associated with the token which
** is popped from the stack, then call it.
**
** Return the major token number for the symbol popped.
*/
static int yy_pop_parser_stack(yyParser *pParser){
  YYCODETYPE yymajor;
  yyStackEntry *yytos = &pParser->yystack[pParser->yyidx];

  if( pParser->yyidx<0 ) return 0;
#ifndef NDEBUG
  if( yyTraceFILE && pParser->yyidx>=0 ){
    fprintf(yyTraceFILE,"%sPopping %s\n",
      yyTracePrompt,
      yyTokenName[yytos->major]);
  }
#endif
  yymajor = yytos->major;
  yy_destructor(pParser, yymajor, &yytos->minor);
  pParser->yyidx--;
  return yymajor;
}

/* 
** Deallocate and destroy a parser.  Destructors are all called for
** all stack elements before shutting the parser down.
**
** Inputs:
** <ul>
** <li>  A pointer to the parser.  This should be a pointer
**       obtained from userdictParseAlloc.
** <li>  A pointer to a function used to reclaim memory obtained
**       from malloc.
** </ul>
*/
void userdictParseFree(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
){
  yyParser *pParser = (yyParser*)p;
  if( pParser==0 ) return;
  while( pParser->yyidx>=0 ) yy_pop_parser_stack(pParser);
#if YYSTACKDEPTH<=0
  free(pParser->yystack);
#endif
  (*freeProc)((void*)pParser);
}

/*
** Return the peak depth of the stack for a parser.
*/
#ifdef YYTRACKMAXSTACKDEPTH
int userdictParseStackPeak(void *p){
  yyParser *pParser = (yyParser*)p;
  return pParser->yyidxMax;
}
#endif

/*
** Find the appropriate action for a parser given the terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_shift_action(
  yyParser *pParser,        /* The parser */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
  int stateno = pParser->yystack[pParser->yyidx].stateno;
 
  if( stateno>YY_SHIFT_COUNT
   || (i = yy_shift_ofst[stateno])==YY_SHIFT_USE_DFLT ){
    return yy_default[stateno];
  }
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
  if( i<0 || i>=YY_ACTTAB_COUNT || yy_lookahead[i]!=iLookAhead ){
    if( iLookAhead>0 ){
#ifdef YYFALLBACK
      YYCODETYPE iFallback;            /* Fallback token */
      if( iLookAhead<sizeof(yyFallback)/sizeof(yyFallback[0])
             && (iFallback = yyFallback[iLookAhead])!=0 ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE, "%sFALLBACK %s => %s\n",
             yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[iFallback]);
        }
#endif
        return yy_find_shift_action(pParser, iFallback);
      }
#endif
#ifdef YYWILDCARD
      {
        int j = i - iLookAhead + YYWILDCARD;
        if( 
#if YY_SHIFT_MIN+YYWILDCARD<0
          j>=0 &&
#endif
#if YY_SHIFT_MAX+YYWILDCARD>=YY_ACTTAB_COUNT
          j<YY_ACTTAB_COUNT &&
#endif
          yy_lookahead[j]==YYWILDCARD
        ){
#ifndef NDEBUG
          if( yyTraceFILE ){
            fprintf(yyTraceFILE, "%sWILDCARD %s => %s\n",
               yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[YYWILDCARD]);
          }
#endif /* NDEBUG */
          return yy_action[j];
        }
      }
#endif /* YYWILDCARD */
    }
    return yy_default[stateno];
  }else{
    return yy_action[i];
  }
}

/*
** Find the appropriate action for a parser given the non-terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_reduce_action(
  int stateno,              /* Current state number */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
#ifdef YYERRORSYMBOL
  if( stateno>YY_REDUCE_COUNT ){
    return yy_default[stateno];
  }
#else
  assert( stateno<=YY_REDUCE_COUNT );
#endif
  i = yy_reduce_ofst[stateno];
  assert( i!=YY_REDUCE_USE_DFLT );
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
#ifdef YYERRORSYMBOL
  if( i<0 || i>=YY_ACTTAB_COUNT || yy_lookahead[i]!=iLookAhead ){
    return yy_default[stateno];
  }
#else
  assert( i>=0 && i<YY_ACTTAB_COUNT );
  assert( yy_lookahead[i]==iLookAhead );
#endif
  return yy_action[i];
}

/*
** The following routine is called if the stack overflows.
*/
static void yyStackOverflow(yyParser *yypParser, YYMINORTYPE *yypMinor){
   userdictParseARG_FETCH;
   yypParser->yyidx--;
#ifndef NDEBUG
   if( yyTraceFILE ){
     fprintf(yyTraceFILE,"%sStack Overflow!\n",yyTracePrompt);
   }
#endif
   while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
   /* Here code is inserted which will execute if the parser
   ** stack every overflows */
   userdictParseARG_STORE; /* Suppress warning about unused %extra_argument var */
}

/*
** Perform a shift action.
*/
static void yy_shift(
  yyParser *yypParser,          /* The parser to be shifted */
  int yyNewState,               /* The new state to shift in */
  int yyMajor,                  /* The major token to shift in */
  YYMINORTYPE *yypMinor         /* Pointer to the minor token to shift in */
){
  yyStackEntry *yytos;
  yypParser->yyidx++;
#ifdef YYTRACKMAXSTACKDEPTH
  if( yypParser->yyidx>yypParser->yyidxMax ){
    yypParser->yyidxMax = yypParser->yyidx;
  }
#endif
#if YYSTACKDEPTH>0 
  if( yypParser->yyidx>=YYSTACKDEPTH ){
    yyStackOverflow(yypParser, yypMinor);
    return;
  }
#else
  if( yypParser->yyidx>=yypParser->yystksz ){
    yyGrowStack(yypParser);
    if( yypParser->yyidx>=yypParser->yystksz ){
      yyStackOverflow(yypParser, yypMinor);
      return;
    }
  }
#endif
  yytos = &yypParser->yystack[yypParser->yyidx];
  yytos->stateno = (YYACTIONTYPE)yyNewState;
  yytos->major = (YYCODETYPE)yyMajor;
  yytos->minor = *yypMinor;
#ifndef NDEBUG
  if( yyTraceFILE && yypParser->yyidx>0 ){
    int i;
    fprintf(yyTraceFILE,"%sShift %d\n",yyTracePrompt,yyNewState);
    fprintf(yyTraceFILE,"%sStack:",yyTracePrompt);
    for(i=1; i<=yypParser->yyidx; i++)
      fprintf(yyTraceFILE," %s",yyTokenName[yypParser->yystack[i].major]);
    fprintf(yyTraceFILE,"\n");
  }
#endif
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static const struct {
  YYCODETYPE lhs;         /* Symbol on the left-hand side of the rule */
  unsigned char nrhs;     /* Number of right-hand side symbols in the rule */
} yyRuleInfo[] = {
  { 54, 2 },
  { 54, 1 },
  { 19, 1 },
  { 19, 1 },
  { 20, 3 },
  { 21, 5 },
  { 22, 3 },
  { 22, 1 },
  { 23, 1 },
  { 23, 0 },
  { 24, 1 },
  { 24, 0 },
  { 25, 1 },
  { 25, 3 },
  { 26, 3 },
  { 26, 1 },
  { 27, 2 },
  { 28, 2 },
  { 28, 1 },
  { 29, 1 },
  { 29, 1 },
  { 29, 1 },
  { 30, 5 },
  { 31, 0 },
  { 31, 1 },
  { 31, 1 },
  { 31, 1 },
  { 40, 3 },
  { 41, 3 },
  { 41, 1 },
  { 42, 1 },
  { 42, 0 },
  { 43, 2 },
  { 43, 1 },
  { 44, 1 },
  { 44, 1 },
  { 33, 1 },
  { 33, 1 },
  { 32, 3 },
  { 32, 1 },
  { 34, 3 },
  { 34, 3 },
  { 35, 2 },
  { 36, 1 },
  { 39, 0 },
  { 39, 1 },
  { 39, 1 },
  { 37, 2 },
  { 37, 1 },
  { 38, 1 },
  { 38, 1 },
  { 45, 1 },
  { 45, 1 },
  { 46, 1 },
  { 48, 1 },
  { 48, 1 },
  { 47, 1 },
  { 49, 1 },
  { 51, 2 },
  { 51, 1 },
  { 50, 1 },
  { 52, 1 },
  { 53, 1 },
  { 55, 0 },
  { 55, 1 },
};

static void yy_accept(yyParser*);  /* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
*/
static void yy_reduce(
  yyParser *yypParser,         /* The parser */
  int yyruleno                 /* Number of the rule by which to reduce */
){
  int yygoto;                     /* The next state */
  int yyact;                      /* The next action */
  YYMINORTYPE yygotominor;        /* The LHS of the rule reduced */
  yyStackEntry *yymsp;            /* The top of the parser's stack */
  int yysize;                     /* Amount to pop the stack */
  userdictParseARG_FETCH;
  yymsp = &yypParser->yystack[yypParser->yyidx];
#ifndef NDEBUG
  if( yyTraceFILE && yyruleno>=0 
        && yyruleno<(int)(sizeof(yyRuleName)/sizeof(yyRuleName[0])) ){
    fprintf(yyTraceFILE, "%sReduce [%s].\n", yyTracePrompt,
      yyRuleName[yyruleno]);
  }
#endif /* NDEBUG */

  /* Silence complaints from purify about yygotominor being uninitialized
  ** in some cases when it is copied into the stack after the following
  ** switch.  yygotominor is uninitialized when a rule reduces that does
  ** not set the value of its left-hand side nonterminal.  Leaving the
  ** value of the nonterminal uninitialized is utterly harmless as long
  ** as the value is never used.  So really the only thing this code
  ** accomplishes is to quieten purify.  
  **
  ** 2007-01-16:  The wireshark project (www.wireshark.org) reports that
  ** without this code, their parser segfaults.  I'm not sure what there
  ** parser is doing to make this happen.  This is the second bug report
  ** from wireshark this week.  Clearly they are stressing Lemon in ways
  ** that it has not been previously stressed...  (SQLite ticket #2172)
  */
  /*memset(&yygotominor, 0, sizeof(yygotominor));*/
  yygotominor = yyzerominor;


  switch( yyruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  #line <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  #line <lineno> <thisfile>
  **     break;
  */
      case 0: /* line ::= rule NEWLINE */
#line 102 "userdict_parser.g"
{
  if(!ps->error) ps->result->extend(yymsp[-1].minor.yy98);
  delete yymsp[-1].minor.yy98;
  yy_destructor(yypParser,1,&yymsp[0].minor);
}
#line 900 "userdict_parser.c"
        break;
      case 1: /* line ::= NEWLINE */
#line 107 "userdict_parser.g"
{
  yy_destructor(yypParser,1,&yymsp[0].minor);
}
#line 907 "userdict_parser.c"
        break;
      case 2: /* rule ::= collocation_rule */
      case 3: /* rule ::= symbol_rule */ yytestcase(yyruleno==3);
      case 7: /* tokens ::= token */ yytestcase(yyruleno==7);
      case 12: /* token ::= words */ yytestcase(yyruleno==12);
      case 15: /* words ::= word */ yytestcase(yyruleno==15);
      case 18: /* inline_rewrites ::= inline_rewrite */ yytestcase(yyruleno==18);
      case 19: /* inline_rewrite ::= inline_replacement */ yytestcase(yyruleno==19);
      case 20: /* inline_rewrite ::= subpattern */ yytestcase(yyruleno==20);
      case 21: /* inline_rewrite ::= stress_mark */ yytestcase(yyruleno==21);
      case 24: /* word_flags ::= stressed_syl_numbers */ yytestcase(yyruleno==24);
      case 25: /* word_flags ::= unstressed_flag */ yytestcase(yyruleno==25);
      case 26: /* word_flags ::= initialism_flag */ yytestcase(yyruleno==26);
      case 29: /* set_elements ::= set_element */ yytestcase(yyruleno==29);
      case 30: /* set_element ::= pattern */ yytestcase(yyruleno==30);
      case 33: /* pattern ::= subpattern */ yytestcase(yyruleno==33);
      case 34: /* subpattern ::= substring */ yytestcase(yyruleno==34);
      case 35: /* subpattern ::= set */ yytestcase(yyruleno==35);
      case 36: /* symbol_pron ::= spaced_symbol_pron */ yytestcase(yyruleno==36);
      case 37: /* symbol_pron ::= token_pron */ yytestcase(yyruleno==37);
      case 39: /* token_pron ::= word_pron */ yytestcase(yyruleno==39);
      case 43: /* pron_repr ::= text_pron_repr */ yytestcase(yyruleno==43);
      case 45: /* word_pron_flags ::= stressed_syl_numbers */ yytestcase(yyruleno==45);
      case 46: /* word_pron_flags ::= unstressed_flag */ yytestcase(yyruleno==46);
      case 48: /* text_pron_repr ::= text_pron_repr_atom */ yytestcase(yyruleno==48);
      case 49: /* text_pron_repr_atom ::= insertion */ yytestcase(yyruleno==49);
      case 50: /* text_pron_repr_atom ::= stress_mark */ yytestcase(yyruleno==50);
      case 59: /* stressed_syl_numbers ::= stressed_syl_number */ yytestcase(yyruleno==59);
#line 110 "userdict_parser.g"
{
  yygotominor.yy98=yymsp[0].minor.yy98;
}
#line 940 "userdict_parser.c"
        break;
      case 4: /* collocation_rule ::= prefix tokens suffix */
#line 120 "userdict_parser.g"
{
  yygotominor.yy98=yymsp[-2].minor.yy98;
  yygotominor.yy98->append(yymsp[-1].minor.yy98);
  yygotominor.yy98->append(yymsp[0].minor.yy98);
  delete yymsp[-1].minor.yy98;
  delete yymsp[0].minor.yy98;
}
#line 951 "userdict_parser.c"
        break;
      case 5: /* symbol_rule ::= symbol space EQUALS space symbol_pron */
#line 129 "userdict_parser.g"
{
  yygotominor.yy98=RHVoice::userdict::ruleset::create<RHVoice::userdict::start_of_token>();
  yygotominor.yy98->append(yymsp[-4].minor.yy98);
  yygotominor.yy98->append(yymsp[0].minor.yy98);
  yygotominor.yy98->append<RHVoice::userdict::end_of_token>();
  delete yymsp[-4].minor.yy98;
  delete yymsp[0].minor.yy98;
  yy_destructor(yypParser,2,&yymsp[-2].minor);
}
#line 964 "userdict_parser.c"
        break;
      case 6: /* tokens ::= tokens SPACE token */
#line 139 "userdict_parser.g"
{
  yygotominor.yy98=yymsp[-2].minor.yy98;
  yygotominor.yy98->append<RHVoice::userdict::end_of_token>();
  yygotominor.yy98->append<RHVoice::userdict::start_of_token>();
  yygotominor.yy98->append(yymsp[0].minor.yy98);
  delete yymsp[0].minor.yy98;
  yy_destructor(yypParser,3,&yymsp[-1].minor);
}
#line 976 "userdict_parser.c"
        break;
      case 8: /* prefix ::= STAR */
      case 10: /* suffix ::= STAR */ yytestcase(yyruleno==10);
#line 153 "userdict_parser.g"
{
  yygotominor.yy98=new RHVoice::userdict::ruleset;
  yy_destructor(yypParser,4,&yymsp[0].minor);
}
#line 985 "userdict_parser.c"
        break;
      case 9: /* prefix ::= */
#line 158 "userdict_parser.g"
{
  yygotominor.yy98=RHVoice::userdict::ruleset::create<RHVoice::userdict::start_of_token>();
}
#line 992 "userdict_parser.c"
        break;
      case 11: /* suffix ::= */
#line 168 "userdict_parser.g"
{
  yygotominor.yy98=RHVoice::userdict::ruleset::create<RHVoice::userdict::end_of_token>();
}
#line 999 "userdict_parser.c"
        break;
      case 13: /* token ::= deletion EQUALS token_pron */
#line 178 "userdict_parser.g"
{
  yygotominor.yy98=yymsp[-2].minor.yy98;
  yygotominor.yy98->append(yymsp[0].minor.yy98);
  delete yymsp[0].minor.yy98;
  yy_destructor(yypParser,2,&yymsp[-1].minor);
}
#line 1009 "userdict_parser.c"
        break;
      case 14: /* words ::= words AND word */
      case 38: /* token_pron ::= token_pron AND word_pron */ yytestcase(yyruleno==38);
#line 185 "userdict_parser.g"
{
  yygotominor.yy98=yymsp[-2].minor.yy98;
  yygotominor.yy98->append<RHVoice::userdict::word_break>();
  yygotominor.yy98->append(yymsp[0].minor.yy98);
  delete yymsp[0].minor.yy98;
  yy_destructor(yypParser,5,&yymsp[-1].minor);
}
#line 1021 "userdict_parser.c"
        break;
      case 16: /* word ::= inline_rewrites word_flags */
      case 17: /* inline_rewrites ::= inline_rewrites inline_rewrite */ yytestcase(yyruleno==17);
      case 32: /* pattern ::= pattern subpattern */ yytestcase(yyruleno==32);
      case 42: /* word_pron ::= pron_repr word_pron_flags */ yytestcase(yyruleno==42);
      case 47: /* text_pron_repr ::= text_pron_repr text_pron_repr_atom */ yytestcase(yyruleno==47);
      case 58: /* stressed_syl_numbers ::= stressed_syl_numbers stressed_syl_number */ yytestcase(yyruleno==58);
#line 198 "userdict_parser.g"
{
  yygotominor.yy98=yymsp[-1].minor.yy98;
  yygotominor.yy98->append(yymsp[0].minor.yy98);
  delete yymsp[0].minor.yy98;
}
#line 1035 "userdict_parser.c"
        break;
      case 22: /* inline_replacement ::= LPAREN deletion EQUALS text_pron_repr RPAREN */
#line 232 "userdict_parser.g"
{
  yygotominor.yy98=yymsp[-3].minor.yy98;
  yygotominor.yy98->append(yymsp[-1].minor.yy98);
  delete yymsp[-1].minor.yy98;
  yy_destructor(yypParser,6,&yymsp[-4].minor);
  yy_destructor(yypParser,2,&yymsp[-2].minor);
  yy_destructor(yypParser,7,&yymsp[0].minor);
}
#line 1047 "userdict_parser.c"
        break;
      case 23: /* word_flags ::= */
#line 239 "userdict_parser.g"
{
  yygotominor.yy98=new RHVoice::userdict::ruleset;
}
#line 1054 "userdict_parser.c"
        break;
      case 27: /* set ::= LBRACE set_elements RBRACE */
#line 259 "userdict_parser.g"
{
  yygotominor.yy98=yymsp[-1].minor.yy98;
  yy_destructor(yypParser,8,&yymsp[-2].minor);
  yy_destructor(yypParser,9,&yymsp[0].minor);
}
#line 1063 "userdict_parser.c"
        break;
      case 28: /* set_elements ::= set_elements COMMA set_element */
#line 264 "userdict_parser.g"
{
  yygotominor.yy98=yymsp[-2].minor.yy98;
  yygotominor.yy98->extend(yymsp[0].minor.yy98);
  delete yymsp[0].minor.yy98;
  yy_destructor(yypParser,10,&yymsp[-1].minor);
}
#line 1073 "userdict_parser.c"
        break;
      case 31: /* set_element ::= */
#line 281 "userdict_parser.g"
{
  yygotominor.yy98=RHVoice::userdict::ruleset::create<RHVoice::userdict::empty_string>();
}
#line 1080 "userdict_parser.c"
        break;
      case 40: /* spaced_symbol_pron ::= spaced_symbol_pron SPACE word_pron */
      case 41: /* spaced_symbol_pron ::= word_pron SPACE word_pron */ yytestcase(yyruleno==41);
#line 331 "userdict_parser.g"
{
  yygotominor.yy98=yymsp[-2].minor.yy98;
  yygotominor.yy98->append<RHVoice::userdict::word_break>();
  yygotominor.yy98->append(yymsp[0].minor.yy98);
  delete yymsp[0].minor.yy98;
  yy_destructor(yypParser,3,&yymsp[-1].minor);
}
#line 1092 "userdict_parser.c"
        break;
      case 44: /* word_pron_flags ::= */
#line 359 "userdict_parser.g"
{
  yygotominor.yy98= new RHVoice::userdict::ruleset;
}
#line 1099 "userdict_parser.c"
        break;
      case 51: /* substring ::= NATIVE_LETTERS */
      case 52: /* substring ::= ENGLISH_LETTERS */ yytestcase(yyruleno==52);
#line 396 "userdict_parser.g"
{
  yygotominor.yy98=RHVoice::userdict::ruleset::create<RHVoice::userdict::substring>(yymsp[0].minor.yy0);
  delete yymsp[0].minor.yy0;
}
#line 1108 "userdict_parser.c"
        break;
      case 53: /* symbol ::= SYM */
#line 408 "userdict_parser.g"
{
  yygotominor.yy98=RHVoice::userdict::ruleset::create<RHVoice::userdict::symbol>(yymsp[0].minor.yy0);
  delete yymsp[0].minor.yy0;
}
#line 1116 "userdict_parser.c"
        break;
      case 54: /* deletion ::= NATIVE_LETTERS */
      case 55: /* deletion ::= ENGLISH_LETTERS */ yytestcase(yyruleno==55);
#line 414 "userdict_parser.g"
{
  yygotominor.yy98=RHVoice::userdict::ruleset::create<RHVoice::userdict::deletion>(yymsp[0].minor.yy0);
  delete yymsp[0].minor.yy0;
}
#line 1125 "userdict_parser.c"
        break;
      case 56: /* insertion ::= NATIVE_LETTERS */
#line 426 "userdict_parser.g"
{
  yygotominor.yy98=RHVoice::userdict::ruleset::create<RHVoice::userdict::insertion>(yymsp[0].minor.yy0);
  delete yymsp[0].minor.yy0;
}
#line 1133 "userdict_parser.c"
        break;
      case 57: /* stress_mark ::= PLUS */
#line 432 "userdict_parser.g"
{
  yygotominor.yy98=RHVoice::userdict::ruleset::create<RHVoice::userdict::stress_mark>();
  yy_destructor(yypParser,14,&yymsp[0].minor);
}
#line 1141 "userdict_parser.c"
        break;
      case 60: /* stressed_syl_number ::= STRESSED_SYL_NUMBER */
#line 449 "userdict_parser.g"
{
  yygotominor.yy98=RHVoice::userdict::ruleset::create<RHVoice::userdict::stressed_syl_number>(yymsp[0].minor.yy0);
  delete yymsp[0].minor.yy0;
}
#line 1149 "userdict_parser.c"
        break;
      case 61: /* unstressed_flag ::= UNSTRESSED */
#line 455 "userdict_parser.g"
{
  yygotominor.yy98=RHVoice::userdict::ruleset::create<RHVoice::userdict::unstressed_flag>();
  yy_destructor(yypParser,16,&yymsp[0].minor);
}
#line 1157 "userdict_parser.c"
        break;
      case 62: /* initialism_flag ::= INITIALISM */
#line 460 "userdict_parser.g"
{
  yygotominor.yy98=RHVoice::userdict::ruleset::create<RHVoice::userdict::initialism_flag>();
  yy_destructor(yypParser,17,&yymsp[0].minor);
}
#line 1165 "userdict_parser.c"
        break;
      case 64: /* space ::= SPACE */
#line 464 "userdict_parser.g"
{
  yy_destructor(yypParser,3,&yymsp[0].minor);
}
#line 1172 "userdict_parser.c"
        break;
      default:
      /* (63) space ::= */ yytestcase(yyruleno==63);
        break;
  };
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yypParser->yyidx -= yysize;
  yyact = yy_find_reduce_action(yymsp[-yysize].stateno,(YYCODETYPE)yygoto);
  if( yyact < YYNSTATE ){
#ifdef NDEBUG
    /* If we are not debugging and the reduce action popped at least
    ** one element off the stack, then we can push the new element back
    ** onto the stack here, and skip the stack overflow test in yy_shift().
    ** That gives a significant speed improvement. */
    if( yysize ){
      yypParser->yyidx++;
      yymsp -= yysize-1;
      yymsp->stateno = (YYACTIONTYPE)yyact;
      yymsp->major = (YYCODETYPE)yygoto;
      yymsp->minor = yygotominor;
    }else
#endif
    {
      yy_shift(yypParser,yyact,yygoto,&yygotominor);
    }
  }else{
    assert( yyact == YYNSTATE + YYNRULE + 1 );
    yy_accept(yypParser);
  }
}

/*
** The following code executes when the parse fails
*/
#ifndef YYNOERRORRECOVERY
static void yy_parse_failed(
  yyParser *yypParser           /* The parser */
){
  userdictParseARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sFail!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
  userdictParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}
#endif /* YYNOERRORRECOVERY */

/*
** The following code executes when a syntax error first occurs.
*/
static void yy_syntax_error(
  yyParser *yypParser,           /* The parser */
  int yymajor,                   /* The major type of the error token */
  YYMINORTYPE yyminor            /* The minor type of the error token */
){
  userdictParseARG_FETCH;
#define TOKEN (yyminor.yy0)
#line 99 "userdict_parser.g"
ps->error=true;
#line 1237 "userdict_parser.c"
  userdictParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
){
  userdictParseARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sAccept!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser accepts */
  userdictParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "userdictParseAlloc" which describes the current state of the parser.
** The second argument is the major token number.  The third is
** the minor token.  The fourth optional argument is whatever the
** user wants (and specified in the grammar) and is available for
** use by the action routines.
**
** Inputs:
** <ul>
** <li> A pointer to the parser (an opaque structure.)
** <li> The major token number.
** <li> The minor token number.
** <li> An option argument of a grammar-specified type.
** </ul>
**
** Outputs:
** None.
*/
void userdictParse(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  userdictParseTOKENTYPE yyminor       /* The value for the token */
  userdictParseARG_PDECL               /* Optional %extra_argument parameter */
){
  YYMINORTYPE yyminorunion;
  int yyact;            /* The parser action. */
  int yyendofinput;     /* True if we are at the end of input */
#ifdef YYERRORSYMBOL
  int yyerrorhit = 0;   /* True if yymajor has invoked an error */
#endif
  yyParser *yypParser;  /* The parser */

  /* (re)initialize the parser, if necessary */
  yypParser = (yyParser*)yyp;
  if( yypParser->yyidx<0 ){
#if YYSTACKDEPTH<=0
    if( yypParser->yystksz <=0 ){
      /*memset(&yyminorunion, 0, sizeof(yyminorunion));*/
      yyminorunion = yyzerominor;
      yyStackOverflow(yypParser, &yyminorunion);
      return;
    }
#endif
    yypParser->yyidx = 0;
    yypParser->yyerrcnt = -1;
    yypParser->yystack[0].stateno = 0;
    yypParser->yystack[0].major = 0;
  }
  yyminorunion.yy0 = yyminor;
  yyendofinput = (yymajor==0);
  userdictParseARG_STORE;

#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sInput %s\n",yyTracePrompt,yyTokenName[yymajor]);
  }
#endif

  do{
    yyact = yy_find_shift_action(yypParser,(YYCODETYPE)yymajor);
    if( yyact<YYNSTATE ){
      assert( !yyendofinput );  /* Impossible to shift the $ token */
      yy_shift(yypParser,yyact,yymajor,&yyminorunion);
      yypParser->yyerrcnt--;
      yymajor = YYNOCODE;
    }else if( yyact < YYNSTATE + YYNRULE ){
      yy_reduce(yypParser,yyact-YYNSTATE);
    }else{
      assert( yyact == YY_ERROR_ACTION );
#ifdef YYERRORSYMBOL
      int yymx;
#endif
#ifndef NDEBUG
      if( yyTraceFILE ){
        fprintf(yyTraceFILE,"%sSyntax Error!\n",yyTracePrompt);
      }
#endif
#ifdef YYERRORSYMBOL
      /* A syntax error has occurred.
      ** The response to an error depends upon whether or not the
      ** grammar defines an error token "ERROR".  
      **
      ** This is what we do if the grammar does define ERROR:
      **
      **  * Call the %syntax_error function.
      **
      **  * Begin popping the stack until we enter a state where
      **    it is legal to shift the error symbol, then shift
      **    the error symbol.
      **
      **  * Set the error count to three.
      **
      **  * Begin accepting and shifting new tokens.  No new error
      **    processing will occur until three tokens have been
      **    shifted successfully.
      **
      */
      if( yypParser->yyerrcnt<0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yymx = yypParser->yystack[yypParser->yyidx].major;
      if( yymx==YYERRORSYMBOL || yyerrorhit ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE,"%sDiscard input token %s\n",
             yyTracePrompt,yyTokenName[yymajor]);
        }
#endif
        yy_destructor(yypParser, (YYCODETYPE)yymajor,&yyminorunion);
        yymajor = YYNOCODE;
      }else{
         while(
          yypParser->yyidx >= 0 &&
          yymx != YYERRORSYMBOL &&
          (yyact = yy_find_reduce_action(
                        yypParser->yystack[yypParser->yyidx].stateno,
                        YYERRORSYMBOL)) >= YYNSTATE
        ){
          yy_pop_parser_stack(yypParser);
        }
        if( yypParser->yyidx < 0 || yymajor==0 ){
          yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
          yy_parse_failed(yypParser);
          yymajor = YYNOCODE;
        }else if( yymx!=YYERRORSYMBOL ){
          YYMINORTYPE u2;
          u2.YYERRSYMDT = 0;
          yy_shift(yypParser,yyact,YYERRORSYMBOL,&u2);
        }
      }
      yypParser->yyerrcnt = 3;
      yyerrorhit = 1;
#elif defined(YYNOERRORRECOVERY)
      /* If the YYNOERRORRECOVERY macro is defined, then do not attempt to
      ** do any kind of error recovery.  Instead, simply invoke the syntax
      ** error routine and continue going as if nothing had happened.
      **
      ** Applications can set this macro (for example inside %include) if
      ** they intend to abandon the parse upon the first syntax error seen.
      */
      yy_syntax_error(yypParser,yymajor,yyminorunion);
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      yymajor = YYNOCODE;
      
#else  /* YYERRORSYMBOL is not defined */
      /* This is what we do if the grammar does not define ERROR:
      **
      **  * Report an error message, and throw away the input token.
      **
      **  * If the input token is $, then fail the parse.
      **
      ** As before, subsequent error messages are suppressed until
      ** three input tokens have been successfully shifted.
      */
      if( yypParser->yyerrcnt<=0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yypParser->yyerrcnt = 3;
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      if( yyendofinput ){
        yy_parse_failed(yypParser);
      }
      yymajor = YYNOCODE;
#endif
    }
  }while( yymajor!=YYNOCODE && yypParser->yyidx>=0 );
  return;
}
