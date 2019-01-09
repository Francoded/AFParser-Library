//    tokenstream.h
//
//    Base class for TokenStream objects
//
//    Allows flow variables to be a custom object

#ifndef TOKENSTREAM
#define TOKENSTREAM

#include <string>
#include <sstream>

class extraction_error : public std::logic_error {
public:
  explicit extraction_error(const std::string& err)
      : std::logic_error(err)
  { };
};


template<typename InType = int>
class TokenStream
{

  public:
    TokenStream(int tok_code, std::string text, InType *in)
      :
        code_(tok_code),
        text_(text),
        in_(in)
    { }
    int get_code() const
    {
      return code_;
    }
    std::string get_text() const
    {
      return text_;
    }
    InType * get_in() const
    {
      return in_;
    }
    template <typename OutType>
    friend TokenStream<InType>& operator>>(TokenStream<InType>& in, OutType& out)
    {
      std::istringstream(in.text_) >> out;
      return in;
    }
  private:
    int           code_;
    std::string   text_;
    InType       *in_;
};

#endif
