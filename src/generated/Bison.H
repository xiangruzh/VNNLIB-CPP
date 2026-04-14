/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_GRAMMAR_BISON_H_INCLUDED
# define YY_GRAMMAR_BISON_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int grammar_debug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    _ERROR_ = 258,                 /* _ERROR_  */
    _BANGEQ = 259,                 /* _BANGEQ  */
    _LPAREN = 260,                 /* _LPAREN  */
    _SYMB_14 = 261,                /* _SYMB_14  */
    _SYMB_17 = 262,                /* _SYMB_17  */
    _SYMB_15 = 263,                /* _SYMB_15  */
    _SYMB_20 = 264,                /* _SYMB_20  */
    _SYMB_16 = 265,                /* _SYMB_16  */
    _SYMB_19 = 266,                /* _SYMB_19  */
    _SYMB_18 = 267,                /* _SYMB_18  */
    _SYMB_21 = 268,                /* _SYMB_21  */
    _RPAREN = 269,                 /* _RPAREN  */
    _STAR = 270,                   /* _STAR  */
    _PLUS = 271,                   /* _PLUS  */
    _COMMA = 272,                  /* _COMMA  */
    _MINUS = 273,                  /* _MINUS  */
    _LT = 274,                     /* _LT  */
    _LDARROW = 275,                /* _LDARROW  */
    _DEQ = 276,                    /* _DEQ  */
    _GT = 277,                     /* _GT  */
    _GTEQ = 278,                   /* _GTEQ  */
    _LBRACK = 279,                 /* _LBRACK  */
    _RBRACK = 280,                 /* _RBRACK  */
    _KW_and = 281,                 /* _KW_and  */
    _KW_or = 282,                  /* _KW_or  */
    T_Initialized = 283,           /* T_Initialized  */
    T_Number = 284,                /* T_Number  */
    T_OnnxString = 285,            /* T_OnnxString  */
    T_VariableName = 286,          /* T_VariableName  */
    T_VersionToken = 287           /* T_VersionToken  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 50 "/home/matthew/Code/VNNLIB-CPP/src/generated/grammar.y"

  int    _int;
  char   _char;
  double _double;
  char*  _string;
  Initialized* _initialized;
  VersionToken* _versiontoken;
  Number* _number;
  VariableName* _variablename;
  ListNumber* listnumber_;
  TensorShape* tensorshape_;
  ArithExpr* arithexpr_;
  ListArithExpr* listarithexpr_;
  BoolExpr* boolexpr_;
  ListBoolExpr* listboolexpr_;
  Assertion* assertion_;
  ListAssertion* listassertion_;
  ElementType* elementtype_;
  OnnxName* onnxname_;
  InputOption* inputoption_;
  ListInputOption* listinputoption_;
  InputDefinition* inputdefinition_;
  OutputDefinition* outputdefinition_;
  HiddenDefinition* hiddendefinition_;
  ListInputDefinition* listinputdefinition_;
  ListHiddenDefinition* listhiddendefinition_;
  ListOutputDefinition* listoutputdefinition_;
  NetworkEquivalence* networkequivalence_;
  ListNetworkEquivalence* listnetworkequivalence_;
  NetworkDefinition* networkdefinition_;
  ListNetworkDefinition* listnetworkdefinition_;
  Version* version_;
  Query* query_;

#line 131 "Bison.H"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif




int grammar_parse (yyscan_t scanner, YYSTYPE *result);


#endif /* !YY_GRAMMAR_BISON_H_INCLUDED  */
