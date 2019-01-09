//      tokenizer.h
//
//      Base Tokenizer class defines Token and Tokens types

#ifndef TOKENIZER
#define TOKENIZER

#include <iostream>
#include <string>
#include <vector>

class Tokenizer
{
  public:
    struct Token
    {
      Token() : code(0)
      { }
      Token(int code, const char *text, size_t leng, size_t lineno, size_t columno)
        : code(code), text(text, leng), lineno(lineno), columno(columno)
      { }
      int         code;    ///< token code
      std::string text;    ///< token lexeme
      size_t      lineno;  ///< line number of lexeme
      size_t      columno; ///< column number of lexeme
    };
    /// token container
    typedef std::vector<Token> Tokens;
    /// returns true if there is a token at the given position
    virtual bool has_pos(size_t pos)
    {
      return pos < size();
    }
    /// returns token at the specified position in the token container, throws exception when out of bounds
    virtual const Token& at(size_t pos)
    {
      return tokens_.at(pos);
    }
    /// clear token container
    void clear()
    {
      tokens_.clear();
    }
    Tokens tokens_; ///< Token container
 protected:
    /// returns the current size of the token container
    size_t size() const
    {
      return tokens_.size();
    }
    /// add token at the back of the token container
    Tokenizer& push_back(const Token& token)
    {
      tokens_.push_back(token);
      return *this;
    }
    /// emplace token at the back of the token container
    Tokenizer& emplace_back(int code, const char *text, size_t leng, size_t lineno, size_t columno)
    {
      tokens_.emplace_back(code, text, leng, lineno, columno);
      return *this;
    }
  private:
};

#endif
