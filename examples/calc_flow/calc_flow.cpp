#include <iostream>
#include "parser.h"
#include "flextokenizer.h"

// Live Calculator Interpreter (w/ in-flow variables) Example 
int main()
{
  
  // define tokens 
  Parser<> plus('+'), minus('-'), times('*'), divides('/'), num(2);
  
  // define nonterminals 
  Parser<int> line, expr, fact, fact_tail, term, term_tail;

  // define flow variables 
  int a(0), b(0), c(0), d(0);

  // AFG 
  line>>a = expr>>a & '\n'
          | Token('q')
          | Token('Q');

  expr>>a = term>>a & term_tail(a)>>a;
  
  term_tail(b)>>b = -('+' & term>>c & [&]{ d = b + c; } & term_tail(d)>>b 
      | '-' & term>>c & [&]{ d = b - c; } & term_tail(d)>>b);
  
  term>>a = fact>>a & fact_tail(a)>>a;
  
  fact_tail(b)>>b = -('*' & fact>>c & [&]{ d = b * c; } & fact_tail(d)>>b 
      | '/' & fact>>c & [&]{ d = b / c; } & fact_tail(d)>>b);
  
  fact>>a = num>>a | '(' & expr>>a & ')';

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

    /* if we can parse sucessfully starting at position pos,
     * print result, i.e. the output flow variable of the starting
     * nonterminal
     */
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
    tokens.clear();
    pos = 0;
  }

  return 0;
}
