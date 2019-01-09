//      odflextokenizer.h
//
//      Constructs an on-demand (files only) container of Tokens from a Flex spec
//
//      flex lexertest.l
//      cc -c lex.yy.c
//	    c++ -std=c++11 -o lexertest lexertest.cpp lex.yy.o
//
//      or with reentrant (MT-safe) scanner:
//
//      flex -R lexertest.l
//      cc -c lex.yy.c
//	    c++ -std=c++11 -DFLEX_REENTRANT -o lexertest lexertest.cpp lex.yy.o

#ifndef ODFLEXTOKENIZER
#define ODFLEXTOKENIZER

#include "tokenizer.h"

#ifdef FLEX_REENTRANT

/// Flex stuff (reentrant)
typedef void *yyscan_t;
typedef void *YY_BUFFER_STATE;
extern "C" int yylex_init(yyscan_t*);
extern "C" int yylex_destroy(yyscan_t);
extern "C" void yyset_in(FILE*, yyscan_t);
extern "C" const char *yyget_text(yyscan_t);
extern "C" size_t yyget_leng(yyscan_t);
extern "C" int yyget_lineno(yyscan_t);
extern "C" int yylex(yyscan_t);
extern "C" YY_BUFFER_STATE yy_scan_string(const char*, yyscan_t);
extern "C" YY_BUFFER_STATE yy_scan_bytes(const char*, int, yyscan_t);
extern "C" void yy_delete_buffer(YY_BUFFER_STATE, yyscan_t);

#else

/// Flex stuff (non-reentrant)
typedef void *YY_BUFFER_STATE;
extern FILE *yyin;
extern const char *yytext;
extern size_t yyleng;
extern int yylineno;
extern "C" int yylex();
extern "C" YY_BUFFER_STATE yy_scan_string(const char*);
extern "C" YY_BUFFER_STATE yy_scan_bytes(const char*, int);
extern "C" void yy_delete_buffer(YY_BUFFER_STATE);

#endif

class FlexTokenizer : public Tokenizer
{
  public:
    FlexTokenizer(FILE *fd = stdin) : fd_(fd), eof_(false)
    {
#ifdef FLEX_REENTRANT
      yylex_init(&yyscanner);
      yyset_in(fd_, yyscanner);
#else
      yyin = fd_;
#endif
      clear();
    }
    FlexTokenizer(const char *s) : fd_(NULL)
    {
#ifdef FLEX_REENTRANT
      yylex_init(&yyscanner);
      YY_BUFFER_STATE buffer = yy_scan_string(s, yyscanner);
#else
      YY_BUFFER_STATE buffer = yy_scan_string(s);
#endif
      clear();
      while (true)
      {
        int code;
#ifdef FLEX_REENTRANT
        code = yylex(yyscanner);
#else
        code = yylex();
#endif
        if (code <= 0)
          break;
#ifdef FLEX_REENTRANT
        emplace_back(code, yyget_text(yyscanner), yyget_leng(yyscanner), yyget_lineno(yyscanner), 0);
#else
        emplace_back(code, yytext, yyleng, yylineno, 0);
#endif
      }
#ifdef FLEX_REENTRANT
      yy_delete_buffer(buffer, yyscanner);
      yylex_destroy(yyscanner);
#else
      yy_delete_buffer(buffer);
#endif
    }
    FlexTokenizer(const std::string& s) : fd_(NULL), eof_(false)
    {
#ifdef FLEX_REENTRANT
      yylex_init(&yyscanner);
      YY_BUFFER_STATE buffer = yy_scan_bytes(s.c_str(), (int)s.size(), yyscanner);
#else
      YY_BUFFER_STATE buffer = yy_scan_bytes(s.c_str(), (int)s.size());
#endif
      clear();
      while (true)
      {
        int code;
#ifdef FLEX_REENTRANT
        code = yylex(yyscanner);
#else
        code = yylex();
#endif
        if (code <= 0)
          break;
#ifdef FLEX_REENTRANT
        emplace_back(code, yyget_text(yyscanner), yyget_leng(yyscanner), yyget_lineno(yyscanner), 0);
#else
        emplace_back(code, yytext, yyleng, yylineno, 0);
#endif
      }
#ifdef FLEX_REENTRANT
      yy_delete_buffer(buffer, yyscanner);
      yylex_destroy(yyscanner);
#else
      yy_delete_buffer(buffer);
#endif
    }
    virtual bool has_pos(size_t pos)
    {
      if (fd_ && !eof_)
        fill_to(pos);
      return pos < size();
    }
    virtual const Token& at(size_t pos)
    {
      if (fd_ && !eof_)
        fill_to(pos);
      return Tokenizer::at(pos);
    }
  private:
    void fill_to(size_t pos)
    {
      while (pos >= size())
      {
        int code;
#ifdef FLEX_REENTRANT
        code = yylex(yyscanner);
#else
        code = yylex();
#endif
        if (code <= 0)
        {
          eof_ = true;
          break;
        }
#ifdef FLEX_REENTRANT
        emplace_back(code, yyget_text(yyscanner), yyget_leng(yyscanner), yyget_lineno(yyscanner), 0);
#else
        emplace_back(code, yytext, yyleng, yylineno, 0);
#endif
      }
    }
    FILE *fd_;
    bool eof_;
#ifdef FLEX_REENTRANT
    yyscan_t yyscanner;
#endif
};

#endif
