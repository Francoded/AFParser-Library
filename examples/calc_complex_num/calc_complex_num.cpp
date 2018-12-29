#include <iostream>
#include <cstring>
#include <cmath>
#include <sstream>

#include "parser.h"
#include "flextokenizer.h"
#include "tokenstream.h"

#define NUM_TOK_CODE 2

class ComplexNum {
 
  friend std::ostream& operator<<(std::ostream & out, ComplexNum a)
  {
    out << a.real;
    if (a.imag < 0)
      out << " - ";
    else if (a.imag > 0)
      out << " + ";
    if (a.imag != 0)
      out << std::abs(a.imag) << "j";
    return out;
  }

  public:
    // constructors 
    ComplexNum()
      : 
        real(0),
        imag(0)
    { }
    ComplexNum(float r, float i = 0) 
      : 
        real(r),
        imag(i)
    { }
    // arithmetic operator overloads 
    ComplexNum operator*(ComplexNum & arg)
    {
      ComplexNum result;
      result.real = (real * arg.real) + (-1 * imag * arg.imag);
      result.imag = (real * arg.imag) + (imag * arg.real);
      return result;
    }
    ComplexNum operator*=(ComplexNum & arg)
    {
      *this = *this * arg;
      return *this;
    }
    ComplexNum operator/(ComplexNum & arg)
    {
      ComplexNum result;
      result.real = (real * arg.real + imag * arg.imag) 
        / (arg.real * arg.real + arg.imag * arg.imag * -1);
      result.imag = (imag * arg.real - real * arg.imag)
			  / (arg.real * arg.real + arg.imag * arg.imag * -1);
      return result;
    }
    ComplexNum operator/=(ComplexNum & arg)
    {
      *this = *this / arg;
      return *this;
    }
    ComplexNum operator+(ComplexNum & arg)
    {
      ComplexNum result;
      result.real = real + arg.real;
      result.imag = imag + arg.imag;
      return result;
    }
    ComplexNum operator+=(ComplexNum & arg)
    {
      *this = *this + arg;
      return *this;
    }
    ComplexNum operator-(ComplexNum & arg)
    {
      ComplexNum result;
      result.real = real - arg.real;
      result.imag = imag - arg.imag;
      return result;
    }
    ComplexNum operator-=(ComplexNum & arg)
    {
      *this = *this - arg;
      return *this;
    }
    // member data 
    double real;
    double imag;
};

// overload extraction operator of TokenStream class to allow the semantic
// values of tokens to be properly stored into ComplexNum flow variables 
TokenStream<ComplexNum> & operator>>(TokenStream<ComplexNum> & is, ComplexNum & out)
{
  std::string tok = is.get_text();

  if (tok.back() == 'j')
  {
    // token is imaginary 
    tok.pop_back();
    std::stringstream tok_stream(tok);
    out.real = 0;
    tok_stream >> out.imag;
  } else {
    // token is real 
    std::stringstream tok_stream(tok);
    tok_stream >> out.real;
    out.imag = 0;
  }
  
  return is;
}

// Complex Number Live Calculator Interpreter Example 
int main()
{

  // create nonterminals line, expr, fact, and term. create terminal num 
  Parser<ComplexNum> line, expr, fact, term, num(NUM_TOK_CODE);

  // create ComplexNum flow variables 
  ComplexNum a, b;
 
  // AFG  
  line>>a = expr>>a & '\n'
          | !Token('q')
          | !Token('Q');
  
  expr>>a = term>>a & *( '+' & term>>b & [&]{ a += b; } 
      | '-' & term>>b & [&]{ a -= b; });
  
  term>>a = fact>>a & *( '*' & fact>>b & [&]{ a *= b; } 
      | '/' & fact>>b & [&]{ a /= b; } );
  
  fact>>a = '(' & expr>>a & ')' | num>>a;

  // FlexTokenizer will use stdin 
  FlexTokenizer tokens;

  // maintain current position 
  size_t pos = 0;

  while (true)
  {
    // user prompt 
    std::cout << "============================================================";
    std::cout << "\nA calculator that supports:";
    std::cout << "\n\t- integers, doubles, and complex numbers";
    std::cout << "\n\t- +, -, *, /";
    std::cout << "\n\t- parenthesis";
    std::cout << "\nFormat: <expression>";
    std::cout << "\nExamples:\n\t1) 4 + 3j * 6\n\t2) (5 + 2j) * (2 - 3j)\n";
    std::cout << "============================================================";

    std::cout << "\nGive me a mathematical expression.";
    std::cout << "\nEnter q or Q to quit: ";

    // begin parsing 
    if (line.parse(&tokens,&pos))
    {
      if (tokens.at(0).text == "q" || tokens.at(0).text == "Q")
      {
        std::cout << "Goodbye!" << std::endl;
        break;
      }
      std::cout << "Expression computed succesfully!\nResult: " << a << "\n" << std::endl;
      a = ComplexNum();
    }
    else
    {
      std::cout << "Expression computation failed\n" << std::endl;
    }

    // clear input 
    tokens.clear();
    pos = 0;
  }
  return 0;
}
