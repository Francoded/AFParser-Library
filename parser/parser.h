// syntax:
//
// Parser<> nt;
// Parser<InOutType> nt;
// Parser<InType,OutType> nt;
//
// Construction of parser objects given parser objects X and Y:
//
// X & Y                concatenation
// X | Y                alternation
// *X                   repeat
// +X                   nonzero repeat
// -X                   optional
// ~X                   lookahead (match and backtrack)
// !X                   negative lookahead (non-match and backtrack)
// N * X                repeat N times
// N-M * X              repeat N to M times
// [&]{ ... }           action
// 'A'                  a token with code 65 (ASCII value of 'A')
// 65                   a token with code 65
// Token('A')           a token with code 65 (ASCII value of 'A')
// Token(65)            a token with code 65
//
//
// When declaring a named terminal, make sure to always pass a token code
// as an integer or a chacter:
// Parser<> t(token_code); // token_code is an int or char
//
// When declaring (non)terminals, specify in/out types:
// Parser<in-type,out-type> nt, t(tok_code);
// if in-type and out-type are the same:
// Parser<type> nt, t(tok_code);
// if in-type and out-type are int:
// Parser<> nt, t(tok_code);
//
//
// Examples:
// nt(in)>>out = ... & ... | ... & [&]{ ... }
// nt(in)>>out = "token" & ...
//
// nt(in)  = ... & nt(in) & ...
// nt>>out = ... & nt>>out & ...

#ifndef PARSER
#define PARSER

#include <cassert>
#include <functional> // std::function<void()>
#include <vector>
#include <stack>
#include <typeinfo>   // typeid()
#include <utility>    // std::swap(x,y)
#include "debug.h"
#include "parsetree.h"
#include "tokenizer.h"
#include "tokenstream.h"

class parsing_error : public std::logic_error {
public:
  explicit parsing_error(const std::string& err)
      : std::logic_error(err)
  { };
};

class BaseParser
{
  friend class ParserPrinter;

  public:
    // constructors
    BaseParser(char tok)
    :
      tok_code(tok),
        tag_(Tag::TOK),
        min_(1),
        max_(1)
    { }
    BaseParser(int tok)
      :
        tok_code(tok),
        tag_(Tag::TOK),
        min_(1),
        max_(1)
    { }
    template<typename F>
    BaseParser(const F& act)
      :
        tag_(Tag::ACT),
        act_(act),
        min_(1),
        max_(1)
    { }
    // destructor
    virtual ~BaseParser()
    {
      for (auto p : obj_)
      {
        delete p;
      }
    }
    // accessors
    virtual void* get_in() const
    {
      return NULL;
    }
    virtual void* get_out() const
    {
      return NULL;
    }
    virtual const BaseParser* get_def() const
    {
      return this;
    }
    virtual const BaseParser* get_tok() const
    {
      return this;
    }
    virtual int get_tok_code() const
    {
      return 0;
    }
    // operator overloads
    friend BaseParser& operator*(size_t n, BaseParser& arg)
    {
      BaseParser *p = &arg;
      if (arg.tag_ != Tag::SEQ && arg.tag_ != Tag::ALT)
      {
        arg.obj_.push_back(p = new BaseParser(Tag::SEQ));
        p->arg_.push_back(&arg);
      }
      p->min_ = p->max_ = n;
      return *p;
    }
    friend const BaseParser& operator*(size_t n, const BaseParser& arg)
    {
      const BaseParser *p = &arg;
      if (arg.tag_ != Tag::SEQ && arg.tag_ != Tag::ALT)
      {
        p = new BaseParser(Tag::SEQ);
        p->arg_.push_back(p->clone(arg));
        p->obj_.push_back(p);
      }
      p->min_ = p->max_ = n;
      return *p;
    }
    friend BaseParser& operator-(size_t n, BaseParser& arg)
    {
      arg.min_ = n;
      return arg;
    }
    friend const BaseParser& operator-(size_t n, const BaseParser& arg)
    {
      arg.min_ = n;
      return arg;
    }
    friend BaseParser& operator*(BaseParser& arg)
    {
      return 0 - MAX * arg;
    }
    friend const BaseParser& operator*(const BaseParser& arg)
    {
      return 0 - MAX * arg;
    }
    friend BaseParser& operator+(BaseParser& arg)
    {
      return 1 - MAX * arg;
    }
    friend const BaseParser& operator+(const BaseParser& arg)
    {
      return 1 - MAX * arg;
    }
    friend BaseParser& operator-(BaseParser& arg)
    {
      return 0 - 1 * arg;
    }
    friend const BaseParser& operator-(const BaseParser& arg)
    {
      return 0 - 1 * arg;
    }
    friend BaseParser& operator~(BaseParser& arg)
    {
      return 1 - 0 * arg;
    }
    friend const BaseParser& operator~(const BaseParser& arg)
    {
      return 1 - 0 * arg;
    }
    friend BaseParser& operator!(BaseParser& arg)
    {
      return 0 - 0 * arg;
    }
    friend const BaseParser& operator!(const BaseParser& arg)
    {
      return 0 - 0 * arg;
    }
    friend BaseParser& operator&(BaseParser& arg1, BaseParser& arg2)
    {
      BaseParser *p = &arg1;
      if (arg1.tag_ != Tag::SEQ || arg1.min_ != 1 || arg1.max_ != 1)
      {
        arg1.obj_.push_back(p = new BaseParser(Tag::SEQ));
        p->arg_.push_back(&arg1);
      }
      p->arg_.push_back(&arg2);
      return *p;
    }
    friend BaseParser& operator&(const BaseParser& arg1, BaseParser& arg2)
    {
      return *arg2.clone(arg1) & arg2;
    }
    friend BaseParser& operator&(BaseParser& arg1, const BaseParser& arg2)
    {
      return arg1 & *arg1.clone(arg2);
    }
    friend const BaseParser& operator&(const BaseParser& arg1, const BaseParser& arg2)
    {
      const BaseParser *p = new BaseParser(Tag::SEQ);
      p->arg_.push_back(p->clone(arg1));
      p->arg_.push_back(p->clone(arg2));
      p->obj_.push_back(p);
      return *p;
    }
    friend BaseParser& operator|(BaseParser& arg1, BaseParser& arg2)
    {
      BaseParser *p = &arg1;
      if (arg1.tag_ != Tag::ALT || arg1.min_ != 1 || arg1.max_ != 1)
      {
        arg1.obj_.push_back(p = new BaseParser(Tag::ALT));
        p->arg_.push_back(&arg1);
      }
      p->arg_.push_back(&arg2);
      return *p;
    }
    friend BaseParser& operator|(const BaseParser& arg1, BaseParser& arg2)
    {
      return *arg2.clone(arg1) | arg2;
    }
    friend BaseParser& operator|(BaseParser& arg1, const BaseParser& arg2)
    {
      return arg1 | *arg1.clone(arg2);
    }
    friend BaseParser& operator|(const BaseParser& arg1, const BaseParser& arg2)
    {
      BaseParser *p = new BaseParser(Tag::ALT);
      p->arg_.push_back(p->clone(arg1));
      p->arg_.push_back(p->clone(arg2));
      p->obj_.push_back(p);
      return *p;
    }
    friend BaseParser& operator&(int tok, BaseParser& arg)
    {
      BaseParser temp(tok);
      return *arg.clone(temp) & arg;
    }
    friend BaseParser& operator&(int tok, const BaseParser& arg)
    {
      BaseParser temp(tok);
      BaseParser *p = arg.clone();
      return *p & *p->clone(temp);
    }
    friend BaseParser& operator&(BaseParser& arg, int tok)
    {
      BaseParser temp(tok);
      return arg & *arg.clone(temp);
    }
    friend BaseParser& operator&(const BaseParser& arg, int tok)
    {
      BaseParser temp(tok);
      BaseParser *p = arg.clone();
      return *p & *p->clone(temp);
    }
    friend BaseParser& operator|(int tok, BaseParser& arg)
    {
      BaseParser temp(tok);
      return *arg.clone(temp) | arg;
    }
    friend BaseParser& operator|(int tok, const BaseParser& arg)
    {
      BaseParser temp(tok);
      BaseParser *p = arg.clone();
      return *p->clone(temp) | *p;
    }
    friend BaseParser& operator|(BaseParser& arg, int tok)
    {
      BaseParser temp(tok);
      return arg | *arg.clone(temp);
    }
    friend BaseParser& operator|(const BaseParser& arg, int tok)
    {
      BaseParser temp(tok);
      BaseParser *p = arg.clone();
      return arg | *p->clone(temp);
    }
    // parsing engine
    virtual bool parse(size_t& pos, Tokenizer *tokens, ParseTree *tree = NULL)
    { 
      
      if (tag_ == Tag::ACT)
      {
        // execute action closure (ignore repeats/optional)
        try {
          act_();
        } catch (parsing_error&) { return false; }

        return true;
      }
      if (tag_ == Tag::SEQ)
      {
        if (max_ > 0)
        {
          // parse sequence X&Y with repeats/optional *(X&Y), +(X&Y), -(X&Y)
          for (size_t k = 0; k < max_; ++k)
          {
            size_t p = pos;
            std::vector<ParseTree> children;
            for (auto a : arg_)
            {
              ParseTree child;
              if (!a->parse(pos, tokens, tree ? &child : NULL))
              {
                if (k < min_)
                {
                  return false;
                } 
                pos = p;
                DBGLOG("SEQ PASSED");
                return true;
              } else if (tree) {
                // Store child temp until all args of SEQ have passed
                if (child.has_parent())
                  children.emplace_back(child);
                else if (child.get_children()->size() > 0)
                  for (auto x : *child.get_children())
                    children.emplace_back(x);
              }
            }
            // Store children saved in tree because all args in SEQ passed
            if (tree)
              for (auto child : children)
                tree->add_child(child);
          }
          return true;
        }
        else
        {
          bool ok = min_ > 0; // (negative) lookahead
          // lookahead sequence (X&Y)
          size_t p = pos;
          std::vector<ParseTree> children;
          for (auto a : arg_)
          {
            ParseTree child;
            if (!a->parse(pos, tokens, tree ? &child : NULL))
            {
              pos = p;
              return !ok;
            } else if (ok && tree) {
              // Store child temp until all args of SEQ have passed
              if (child.has_parent())
                children.emplace_back(child);
              else if (child.get_children()->size() > 0)
                for (auto x : *child.get_children())
                  children.emplace_back(x);
            }
          }
          if (ok && tree)
            for (auto child : children)
              tree->add_child(child);
          pos = p;
          return ok;
        }
      }
      if (tag_ == Tag::ALT)
      {
        if (max_ > 0)
        {
          // parse alternations (X|Y) with repeats/optional *(X|Y), +(X|Y), -(X|Y)
          for (size_t k = 0; k < max_; ++k)
          {
            size_t p = pos;
            ParseTree child;
            for (auto a : arg_)
            {
              pos = p; 
              child = ParseTree();
              if (a->parse(pos, tokens, tree ? &child : NULL))
              {
                if (tree)
                {
                  // else if handles repeats
                  if (child.has_parent())
                    tree->add_child(child);
                  else if (child.get_children()->size() > 0)
                    for (auto x : *child.get_children())
                      tree->add_child(x);
                }
                goto next; // continue outer loop
              }
            }
            if (k < min_)
              return false;
            return true;
          next:
            continue;
          }
          return true;
        }
        else
        {
          bool ok = min_ > 0; // (negative) lookahead
          // lookahead alternations (X|Y)
          size_t p = pos;
          for (auto a : arg_)
          {
            pos = p;
            ParseTree child;
            if (a->parse(pos, tokens, tree ? &child : NULL))
            {
              pos = p;
              if (ok && tree)
              {
                // else if handles repeats
                if (child.has_parent())
                  tree->add_child(child);
                else if (child.get_children()->size() > 0)
                  for (auto x : *child.get_children())
                    tree->add_child(x);
              }
              return ok;
            }
          }
          pos = p;
          return !ok;
        }
      }
      if (tag_ == Tag::TOK)
      {
        if (tokens->has_pos(pos))
        if (tokens->has_pos(pos) && tokens->at(pos).code == tok_code)
        {
          if (tree)
          {
            tree->set_parent(tokens->at(pos).text);
          }
          ++pos;
          return true;
        }
        return false; 
      }
      return false;
    }
  protected:
    
    enum class Tag { DEF, NON, TOK, ACT, SEQ, ALT };
    typedef std::function<void()> Action;
    static const size_t MAX = ~static_cast<size_t>(0);
    
    // constructors
    explicit BaseParser(Tag tag)
      :
        tag_(tag),
        min_(1),
        max_(1)
    { }
    explicit BaseParser(const BaseParser& arg)
      :
        tok_code(arg.tok_code),
        tag_(arg.tag_),
        act_(arg.act_),
        min_(arg.min_),
        max_(arg.max_)
    {
      std::swap(arg_, arg.arg_); // arg loses all its args
      std::swap(obj_, arg.obj_); // delegate deletion to the new object
    }

    // helper functions
    BaseParser *clone(const BaseParser& arg) const
    {
      BaseParser *p = arg.clone();
      obj_.push_back(p);
      return p;
    }
    virtual BaseParser *clone() const
    {
      BaseParser *p = new BaseParser(*this); // this object loses its args
      return p;
    }
    virtual void save(void *except)
    {
      for (auto a : arg_)
        a->save(except);
    }
    virtual void restore(void *except)
    {
      for (auto a : arg_)
        a->restore(except);
    }

    // member data
    int                                     tok_code; // token code
    Tag                                     tag_; // selector
    Action                                  act_; // action closure
    mutable size_t                          min_; // min of *X and +X repeats (0/1), -X optional (0), ~X and !X lookahead (1/0)
    mutable size_t                          max_; // max of *X and +X repeats (MAX), -X optional (1), ~X and !X lookahead (0)
    mutable std::vector<BaseParser*>        arg_; // arguments of SEQ and ALT
    mutable std::vector<const BaseParser*>  obj_; // collection of clones to delete
};

template<typename InType = int, typename OutType = InType>
class Parser : public BaseParser 
{
  friend class BaseParser;
  public:
    // constructors
    Parser()
      :
        BaseParser(Tag::DEF),
        def_(NULL),
        tok_(NULL),
        in_(NULL),
        out_(NULL)
    { }
    Parser(int tok)
      :
        BaseParser(tok),
        def_(NULL),
        tok_(this),
        in_(NULL),
        out_(NULL)
    {
    }
    explicit Parser(const Parser&) = default;
   
    // accessors
    virtual void* get_in() const
    {
      return in_;
    }
    virtual void* get_out() const
    {
      return out_;
    }
    virtual const Parser* get_def() const
    {
      return def_ ? def_ : this;
    }
    virtual const Parser* get_tok() const
    {
      return tok_ ? tok_ : this;
    }
    virtual int get_tok_code() const
    {
      return tok_code;
    }
    static size_t lineno(Tokenizer *tokens, size_t pos)
    {
      return tokens->at(pos).lineno;
    }
    static size_t columno(Tokenizer *tokens, size_t pos)
    {
      return tokens->at(pos).columno;
    }
    
    // operator overloads
    Parser& operator()(InType& in)
    {
      assert(tag_ == Tag::DEF || tag_ == Tag::TOK);
      Parser *p = this;
      if (tag_ == Tag::DEF)
        obj_.push_back(p = new Parser(this, &in, out_));
      else if (tag_ == Tag::TOK)
        obj_.push_back(p = new Parser(this, tok_code, &in, out_));
      return *p;
    }
    friend Parser& operator>>(Parser& arg, OutType& out)
    {
      assert(arg.tag_ == Tag::DEF || arg.tag_ == Tag::NON || arg.tag_ == Tag::TOK);
      Parser *p = &arg;
      if (arg.tag_ == Tag::DEF)
        arg.obj_.push_back(p = new Parser(&arg, arg.in_, &out));
      else if (arg.tag_ == Tag::TOK)
        arg.obj_.push_back(p = new Parser(arg.tok_, arg.tok_code, arg.in_, &out));
      p->out_ = &out;
      return *p;
    }
    Parser& operator=(int tok)
    {
      assert(tag_ == Tag::DEF || tag_ == Tag::NON);
      Parser *p = this;
      if (tag_ == Tag::DEF)
      {
        obj_.push_back(p = new Parser(Tag::TOK));
        arg_.push_back(p);
      }
      else if (tag_ == Tag::NON)
      {
        assert(def_ != NULL);
        def_->obj_.push_back(p = new Parser(Tag::TOK));
        def_->arg_.push_back(p);
      }
      p->tok_code = tok;
      return *this;
    }
    Parser& operator=(Parser& rhs)
    {
      return operator=(dynamic_cast<BaseParser&>(rhs)); // could never NULL the pointer
    }
    Parser& operator=(const Parser& rhs)
    {
      return operator=(*clone(rhs));
    }
    Parser& operator=(BaseParser& rhs)
    {
      assert(tag_ == Tag::DEF || tag_ == Tag::NON);
      Parser *lhs = this;
      if (tag_ == Tag::NON)
      {
        lhs = def_;
        if (lhs->in_ == NULL)
          lhs->in_ = in_;
        else
          assert(lhs->in_ == in_); // must use same input arg
        if (lhs->out_ == NULL)
          lhs->out_ = out_;
        else
          assert(lhs->out_ == out_); // must use same output arg
      }
      lhs->arg_.push_back(&rhs);
      return *this;
    }
    Parser& operator=(const BaseParser& rhs)
    {
      return operator=(*clone(rhs));
    }
    
    // parsing engine
    bool parse(Tokenizer *tokens, size_t *pos = NULL, ParseTree *tree = NULL)
    {
      if (pos && !tokens->has_pos(*pos))
        return false;
      size_t p = 0;
      if (tree)
        tree->clear();
      return parse(pos ? *pos : p, tokens, tree);
    }
    virtual bool parse(size_t & pos, Tokenizer *tokens, ParseTree *tree = NULL)
    {
      if (tag_ == Tag::DEF)
      {
        ParseTree child;
        // parse nonterminal definitions (w/o in/out)
        BaseParser::save(out_);
        size_t p = pos;
        for (auto a : arg_)
        {
          pos = p;
          if (a->parse(pos, tokens, tree ? &child : NULL))
          {
            if (tree)
            {
              tree->set_parent(this);
              if (child.has_parent())
                tree->add_child(child);
              else
                for (auto x : *child.get_children())
                  tree->add_child(x);
            }
            BaseParser::restore(out_);
            return true;
          }
        }
        BaseParser::restore(out_);
        tree = NULL;
        return false;
      }
      if (tag_ == Tag::NON)
      {
        // parse nonterminal w/ in/out
        assert(def_ != NULL);
        assert(def_->in_ || def_->out_); // this is a NON, not a DEF that has no IO
        assert(!def_->in_ || in_); // no input arg, when one is required, parhaps default initialize input?
        bool ok;
        if ((void*)def_->in_ == (void*)def_->out_ &&
            typeid(def_->in_) == typeid(def_->out_)) // formal in == out
        {
          OutType tmpo;
          if (def_->out_ != out_)
            std::swap(tmpo, *def_->out_);
          if (def_->in_ != in_)
          {
            std::swap(*def_->in_, *in_);
          }
          ok = def_->parse(pos, tokens, tree);
          if (def_->out_ != out_)
          {
            if (out_)
              std::swap(*out_, *def_->out_);
            std::swap(*def_->out_,  tmpo);
          }
        }
        else
        {
          InType tmpi;
          if (def_->in_ && def_->in_ != in_)
          {
            std::swap(tmpi, *def_->in_);
            std::swap(*def_->in_, *in_);
          }
          OutType tmpo;
          if (def_->out_ && def_->out_ != out_)
            std::swap(tmpo, *def_->out_);
          ok = def_->parse(pos, tokens, tree);
          if (def_->in_ && def_->in_ != in_)
          {
            std::swap(*in_, *def_->in_);
            std::swap(*def_->in_, tmpi);
          }
          if (def_->out_ && def_->out_ != out_)
          {
            if (out_)
              std::swap(*out_, *def_->out_);
            std::swap(*def_->out_,  tmpo);
          }
        }
        return ok;
      }
      if (tag_ == Tag::TOK)
      {
        if (tokens->has_pos(pos))
        if (tokens->has_pos(pos) && tokens->at(pos).code == tok_code)
        {
          if (out_)
          {
            TokenStream<InType> tok_stream(tokens->at(pos).code, tokens->at(pos).text, in_);
            try
            {
              tok_stream >> *out_;
            } catch (extraction_error&) { return false; }
          }
          if (tree)
          {
            tree->set_parent(tokens->at(pos).text);
          }
          ++pos;
          return true;
        }
        return false; 
      }
      return BaseParser::parse(pos, tokens, tree);
    }

  protected:
    // constructors
    Parser(
        Parser  *def,
        InType  *in,
        OutType *out)
      :
        BaseParser(Tag::NON),
        def_(def),
        tok_(NULL),
        in_(in),
        out_(out)
    { }
    Parser(
        Parser      *tok,
        int         code,
        InType      *in,
        OutType     *out)
      :
        BaseParser(code),
        def_(NULL),
        tok_(tok),
        in_(in),
        out_(out)
    { }
    explicit Parser(Tag tag)
      :
        BaseParser(tag)
    { }

    // helper functions
    using BaseParser::clone;
    virtual BaseParser *clone() const
    {
      Parser *p = new Parser(*this);
      return p;
    }
    virtual void save(void *except)
    {
      if (tag_ == Tag::NON || tag_ == Tag::TOK)
      {
        if (out_ && out_ != except)
        {
          stk_.push(*out_);
        }
      }
    }
    virtual void restore(void *except)
    {
      if (tag_ == Tag::NON || tag_ == Tag::TOK)
      {
        if (out_ && out_ != except)
        {
          *out_ = stk_.top();
          stk_.pop();
        }
      }
    }

    // member data
    Parser             *def_;
    Parser             *tok_;
    InType             *in_;
    OutType            *out_;
    std::stack<OutType> stk_;
};

// allows Token('a') and Token(15) syntax
typedef BaseParser Token;
#endif
