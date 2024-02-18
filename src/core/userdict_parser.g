/* Copyright (C) 2013, 2024  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

%include {#include "core/userdict.hpp"}

%name userdictParse

%start_symbol line

%token_prefix UDTK_
%token_type {RHVoice::userdict::token*}
%token_destructor {delete $$;}

%type rule {RHVoice::userdict::ruleset*}
%destructor rule {delete $$;}
%type collocation_rule {RHVoice::userdict::ruleset*}
%destructor collocation_rule {delete $$;}
%type symbol_rule {RHVoice::userdict::ruleset*}
%destructor symbol_rule {delete $$;}
%type tokens {RHVoice::userdict::ruleset*}
%destructor tokens {delete $$;}
%type prefix {RHVoice::userdict::ruleset*}
%destructor prefix {delete $$;}
%type suffix {RHVoice::userdict::ruleset*}
%destructor suffix {delete $$;}
%type token {RHVoice::userdict::ruleset*}
%destructor token {delete $$;}
%type words {RHVoice::userdict::ruleset*}
%destructor words {delete $$;}
%type word {RHVoice::userdict::ruleset*}
%destructor word {delete $$;}
%type inline_rewrites {RHVoice::userdict::ruleset*}
%destructor inline_rewrites {delete $$;}
%type inline_rewrite {RHVoice::userdict::ruleset*}
%destructor inline_rewrite {delete $$;}
%type inline_replacement {RHVoice::userdict::ruleset*}
%destructor inline_replacement {delete $$;}
%type word_flags {RHVoice::userdict::ruleset*}
%destructor word_flags {delete $$;}
%type token_pron {RHVoice::userdict::ruleset*}
%destructor token_pron {delete $$;}
%type symbol_pron {RHVoice::userdict::ruleset*}
%destructor symbol_pron {delete $$;}
%type spaced_symbol_pron {RHVoice::userdict::ruleset*}
%destructor spaced_symbol_pron {delete $$;}
%type word_pron {RHVoice::userdict::ruleset*}
%destructor word_pron {delete $$;}
%type pron_repr {RHVoice::userdict::ruleset*}
%destructor pron_repr {delete $$;}
%type text_pron_repr {RHVoice::userdict::ruleset*}
%destructor text_pron_repr {delete $$;}
%type text_pron_repr_atom {RHVoice::userdict::ruleset*}
%destructor text_pron_repr_atom {delete $$;}
%type word_pron_flags {RHVoice::userdict::ruleset*}
%destructor word_pron_flags {delete $$;}
%type set {RHVoice::userdict::ruleset*}
%destructor set {delete $$;}
%type set_elements {RHVoice::userdict::ruleset*}
%destructor set_elements {delete $$;}
%type set_element {RHVoice::userdict::ruleset*}
%destructor set_element {delete $$;}
%type pattern {RHVoice::userdict::ruleset*}
%destructor pattern {delete $$;}
%type subpattern {RHVoice::userdict::ruleset*}
%destructor subpattern {delete $$;}
%type substring {RHVoice::userdict::ruleset*}
%destructor substring {delete $$;}
%type symbol {RHVoice::userdict::ruleset*}
%destructor symbol {delete $$;}
%type insertion {RHVoice::userdict::ruleset*}
%destructor insertion {delete $$;}
%type deletion {RHVoice::userdict::ruleset*}
%destructor deletion {delete $$;}
%type stress_mark {RHVoice::userdict::ruleset*}
%destructor stress_mark {delete $$;}
%type stressed_syl_number {RHVoice::userdict::ruleset*}
%destructor stressed_syl_number {delete $$;}
%type stressed_syl_numbers {RHVoice::userdict::ruleset*}
%destructor stressed_syl_numbers {delete $$;}
%type unstressed_flag {RHVoice::userdict::ruleset*}
%destructor unstressed_flag {delete $$;}
%type initialism_flag {RHVoice::userdict::ruleset*}
%destructor initialism_flag {delete $$;}
%type foreign_flag {RHVoice::userdict::ruleset*}
%destructor foreign_flag {delete $$;}
%type native_flag {RHVoice::userdict::ruleset*}
%destructor native_flag {delete $$;}

%extra_argument {RHVoice::userdict::parse_state* ps}

%syntax_error {ps->error=true;}

line ::= rule(X) NEWLINE.
{
  if(!ps->error) ps->result->extend(X);
  delete X;
}

line ::= NEWLINE.

rule(X) ::= collocation_rule(Y).
{
  X=Y;
}

rule(X) ::= symbol_rule(Y).
{
  X=Y;
}

collocation_rule(X) ::= prefix(U) tokens(V) suffix(W).
{
  X=U;
  X->append(V);
  X->append(W);
  delete V;
  delete W;
}

symbol_rule(X) ::= symbol(Y) space EQUALS space symbol_pron(Z).
{
  X=RHVoice::userdict::ruleset::create<RHVoice::userdict::start_of_token>();
  X->append(Y);
  X->append(Z);
  X->append<RHVoice::userdict::end_of_token>();
  delete Y;
  delete Z;
}

tokens(X) ::= tokens(Y) SPACE token(Z).
{
  X=Y;
  X->append<RHVoice::userdict::end_of_token>();
  X->append<RHVoice::userdict::start_of_token>();
  X->append(Z);
  delete Z;
}

tokens(X) ::= token(Y).
{
  X=Y;
}

prefix(X) ::= STAR.
{
  X=new RHVoice::userdict::ruleset;
}

prefix(X) ::= .
{
  X=RHVoice::userdict::ruleset::create<RHVoice::userdict::start_of_token>();
}

suffix(X) ::= STAR.
{
  X=new RHVoice::userdict::ruleset;
}

suffix(X) ::= .
{
  X=RHVoice::userdict::ruleset::create<RHVoice::userdict::end_of_token>();
}

token(X) ::= words(Y).
{
  X=Y;
}

token(X) ::= deletion(Y) EQUALS token_pron(Z).
{
  X=Y;
  X->append(Z);
  delete Z;
}

words(X) ::= words(Y) AND word(Z).
{
  X=Y;
  X->append<RHVoice::userdict::word_break>();
  X->append(Z);
  delete Z;
}

words(X) ::= word(Y).
{
  X=Y;
}

word(X) ::= inline_rewrites(Y) word_flags(Z).
{
  X=Y;
  X->append(Z);
  delete Z;
}

inline_rewrites(X) ::= inline_rewrites(Y) inline_rewrite(Z).
{
  X=Y;
  X->append(Z);
  delete Z;
}

inline_rewrites(X) ::= inline_rewrite(Y).
{
  X=Y;
}

inline_rewrite(X) ::= inline_replacement(Y).
{
  X=Y;
}

inline_rewrite(X) ::= subpattern(Y).
{
  X=Y;
}

inline_rewrite(X) ::= stress_mark(Y).
{
  X=Y;
}

inline_replacement(X) ::= LPAREN deletion(Y) EQUALS text_pron_repr(Z) RPAREN.
{
  X=Y;
  X->append(Z);
  delete Z;
}

word_flags(X) ::= .
{
  X=new RHVoice::userdict::ruleset;
}

word_flags(X) ::= stressed_syl_numbers(Y).
{
  X=Y;
}

word_flags(X) ::= unstressed_flag(Y).
{
  X=Y;
}

word_flags(X) ::= initialism_flag(Y).
{
  X=Y;
}

word_flags(X) ::= foreign_flag(Y).
{
  X=Y;
}

word_flags(X) ::= initialism_flag(Y) foreign_flag(Z).
{
  X=Y;
  X->append(Z);
  delete Z;
}

word_flags(X) ::= foreign_flag(Y) initialism_flag(Z).
{
  X=Y;
  X->append(Z);
  delete Z;
}

word_flags(X) ::= native_flag(Y).
{
  X=Y;
}

set(X) ::= LBRACE set_elements(Y) RBRACE.
{
  X=Y;
}

set_elements(X) ::= set_elements(Y) COMMA set_element(Z).
{
  X=Y;
  X->extend(Z);
  delete Z;
}

set_elements(X) ::= set_element(Y).
{
  X=Y;
}

set_element(X) ::= pattern(Y).
{
  X=Y;
}

set_element(X) ::= .
{
  X=RHVoice::userdict::ruleset::create<RHVoice::userdict::empty_string>();
}

pattern(X) ::= pattern(Y) subpattern(Z).
{
  X=Y;
  X->append(Z);
  delete Z;
}

pattern(X) ::= subpattern(Y).
{
  X=Y;
}

subpattern(X) ::= substring(Y).
{
  X=Y;
}

subpattern(X) ::= set(Y).
{
  X=Y;
}

symbol_pron(X) ::= spaced_symbol_pron(Y).
{
  X=Y;
}

symbol_pron(X) ::=  token_pron(Y).
{
  X=Y;
}

token_pron(X) ::= token_pron(Y) AND word_pron(Z).
{
  X=Y;
  X->append<RHVoice::userdict::word_break>();
  X->append(Z);
  delete Z;
}

token_pron(X) ::= word_pron(Y).
{
  X=Y;
}

spaced_symbol_pron(X) ::= spaced_symbol_pron(Y) SPACE word_pron(Z).
{
  X=Y;
  X->append<RHVoice::userdict::word_break>();
  X->append(Z);
  delete Z;
}

spaced_symbol_pron(X) ::= word_pron(Y) SPACE word_pron(Z).
{
  X=Y;
  X->append<RHVoice::userdict::word_break>();
  X->append(Z);
  delete Z;
}

word_pron(X) ::= pron_repr(Y) word_pron_flags(Z).
{
  X=Y;
  X->append(Z);
  delete Z;
}

pron_repr(X) ::= text_pron_repr(Y).
{
  X=Y;
}

word_pron_flags(X) ::= .
{
  X= new RHVoice::userdict::ruleset;
}

word_pron_flags(X) ::= stressed_syl_numbers(Y).
{
  X=Y;
}

word_pron_flags(X) ::= unstressed_flag(Y).
{
  X=Y;
}

word_pron_flags(X) ::= foreign_flag(Y).
{
  X=Y;
}


text_pron_repr(X) ::= text_pron_repr(Y) text_pron_repr_atom(Z).
{
  X=Y;
  X->append(Z);
  delete Z;
}

text_pron_repr(X) ::= text_pron_repr_atom(Y).
{
  X=Y;
}

text_pron_repr_atom(X)::= insertion(Y).
{
  X=Y;
}

text_pron_repr_atom(X) ::= stress_mark(Y).
{
  X=Y;
}

substring(X) ::= NATIVE_LETTERS(Y).
{
  X=RHVoice::userdict::ruleset::create<RHVoice::userdict::substring>(Y);
  delete Y;
}

substring(X) ::= ENGLISH_LETTERS(Y).
{
  X=RHVoice::userdict::ruleset::create<RHVoice::userdict::substring>(Y);
  delete Y;
}

symbol(X) ::= SYM(Y).
{
  X=RHVoice::userdict::ruleset::create<RHVoice::userdict::symbol>(Y);
  delete Y;
}

deletion(X) ::= NATIVE_LETTERS(Y).
{
  X=RHVoice::userdict::ruleset::create<RHVoice::userdict::deletion>(Y);
  delete Y;
}

deletion(X) ::= ENGLISH_LETTERS(Y).
{
  X=RHVoice::userdict::ruleset::create<RHVoice::userdict::deletion>(Y);
  delete Y;
}

insertion(X) ::= NATIVE_LETTERS(Y).
{
  X=RHVoice::userdict::ruleset::create<RHVoice::userdict::insertion>(Y);
  delete Y;
}

stress_mark(X) ::= PLUS.
{
  X=RHVoice::userdict::ruleset::create<RHVoice::userdict::stress_mark>();
}

stressed_syl_numbers(X) ::= stressed_syl_numbers(Y) stressed_syl_number(Z).
{
  X=Y;
  X->append(Z);
  delete Z;
}

stressed_syl_numbers(X) ::= stressed_syl_number(Y).
{
  X=Y;
}

stressed_syl_number(X) ::= STRESSED_SYL_NUMBER(Y).
{
  X=RHVoice::userdict::ruleset::create<RHVoice::userdict::stressed_syl_number>(Y);
  delete Y;
}

unstressed_flag(X) ::= UNSTRESSED.
{
  X=RHVoice::userdict::ruleset::create<RHVoice::userdict::unstressed_flag>();
}

initialism_flag(X) ::= INITIALISM.
{
  X=RHVoice::userdict::ruleset::create<RHVoice::userdict::initialism_flag>();
}

foreign_flag(X) ::= FOREIGN.
{
  X=RHVoice::userdict::ruleset::create<RHVoice::userdict::foreign_flag>();
}

native_flag(X) ::= NATIVE.
{
  X=RHVoice::userdict::ruleset::create<RHVoice::userdict::native_flag>();
}

space ::= . space ::= SPACE.

// Local Variables:
// mode: lemon-c++
// End:
