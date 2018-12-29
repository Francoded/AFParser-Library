#include <iostream>
#include "parser.h"
#include "flextokenizer.h"

// Counting Parser Example
int main()
{
  // declare nonterminal A and integer flow variables x and y
  Parser<int> A;
  int x = 0;

  // AFG
  A>>x = Token('a') & A>>x & [&]{ x++; } | Token('a') & [&]{ x = 1; };

  // get input
  std::cout << "Enter some number of a's to count: ";
  std::string input;
  std::cin >> input;

  // use the FlexTokenizer class to tokenize input string
  FlexTokenizer tokens(input);

  // parse
  if (A.parse(&tokens))
  {
    std::cout << "Parsing Sucessful: " << x << " a's parsed." << std::endl;
  }
  return 0;
}
