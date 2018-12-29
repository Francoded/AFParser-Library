#include <iostream>
#include <map>
#include <string>
#include <cmath>
#include "parser.h"
#include "flextokenizer.h"
#include "tokenstream.h"

typedef std::map<std::string,float> SymbolTable;

TokenStream<std::map<std::string,float>>& operator>>(TokenStream<std::map<std::string,float>> & in, float& out)
{
  // check if symbol 
  if (in.get_code() == 1)
  {
    // check if symbol exists in symbol table 
    if (in.get_in()->find(in.get_text()) != in.get_in()->end())
      out = (*in.get_in())[in.get_text()];
    else
      throw extraction_error("Failed to find variable in symbol table");
  }
  else
    throw std::logic_error("Error in lexer specification");
   
  return in;
}

// Advanced Live Calculator Interperter Example 
int main()
{
  
  // declare symbol table 
  SymbolTable sym;

  // declare (non)terminals 
  Parser<float> line, input, expr, fact, term,
    num(2), log(3), sin(4), cos(5), tan(6);

  Parser<std::string,std::string> new_id(1);
  Parser<std::map<std::string,float>,float> id(1);

  // declare flow variables 
  float a(0), b(0);
  std::string c;
  
  // AFG 
  line>>a = input>>a & '\n'
          | !Token('q')
          | !Token('Q');

  input>>a = *(new_id>>c & '=' & expr>>a & ';' & [&]{ sym[c] = a; })
      & expr>>a & ';';

  expr>>a = term>>a & *( '+' & term>>b & [&]{ a += b; } 
      | '-' & term>>b & [&]{ a -= b; } );
  
  term>>a = fact>>a & *( '*' & fact>>b & [&]{ a *= b; } 
      | '/' & fact>>b & [&]{ a /= b; } );
  
  fact>>a =
    (
      '(' & expr>>a & ')'
      | log & '(' & expr>>a & ')' & [&]{ a = std::log(a); }
      | sin & '(' & expr>>a & ')' & [&]{ a = std::sin(a); }
      | cos & '(' & expr>>a & ')' & [&]{ a = std::cos(a); }
      | tan & '(' & expr>>a & ')' & [&]{ a = std::tan(a); }
      | id(sym)>>a
      | num>>a
    ) & -( '^' & fact>>b & [&]{ a = std::pow(a,b); });

  // FlexTokenizer will use stdin 
  FlexTokenizer tokens;
  
  // maintain current position 
  size_t pos = 0;

  while (true)
  {
    // user prompt 
    std::cout << "============================================================";
    std::cout << "\nA calculator that supports:";
    std::cout << "\n\t- integers, doubles, and symbols\n\t- +, -, *, /";
    std::cout << "\n\t- parenthesis\n\t- natural log\n\t- exponents";
    std::cout << "\n\t- sin, cos, tan";
    std::cout << "\nFormat: <optional_declarations> <final_expression>";
    std::cout << "\nExamples:";
    std::cout << "\n\t1) 5 + 6;\n\t2) x = 5 + 3; y = 6; x * y - 2;\n";
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
      std::cout << "Expression computed successfully!\nResult: " << a << "\n" << std::endl;
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
