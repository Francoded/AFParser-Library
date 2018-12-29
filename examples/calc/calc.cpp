#include <iostream>
#include "parser.h"
#include "flextokenizer.h"

// Calculator Live Interpreter Example
int main()
{
  // define tokens 
  Parser<> plus('+'), minus('-'), times('*'), divides('/'), num(2);
  
  // define nonterminals 
  Parser<int> line, expr, fact, term;

  // define flow variables 
  int a(0), b(0);

  // AFG 
  line>>a = expr>>a & Token('\n')
          | !Token('q')
          | !Token('Q');

  expr>>a = term>>a & *( plus & term>>b & [&]{ a += b; } 
      | minus & term>>b & [&]{ a -= b; } );
  
  term>>a = fact>>a & *( times & fact>>b & [&]{ a *= b; } 
      | divides & fact>>b & [&]{ a /= b; } );
  
  fact>>a = Token('(') & expr>>a & Token(')') | num>>a;

  // FlexTokenizer will use stdin 
  FlexTokenizer tokens;

  // maintain current position 
  size_t pos = 0;

  while (true)
  {
    // user prompt 
    std::cout << "============================================================";
    std::cout << "\nA calculator that supports:";
    std::cout << "\n\t- integers and doubles";
    std::cout << "\n\t- +, -, *, /";
    std::cout << "\n\t- parenthesis";
    std::cout << "\nFormat: <arithmetic expression>";
    std::cout << "\nExamples:\n\t1) 4 + 3 * 2\n\t2) (7 - 2) / 5\n";
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
