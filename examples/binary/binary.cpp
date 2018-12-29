#include <iostream>
#include "parser.h"
#include "parsetree.h"
#include "parserprinter.h"
#include "flextokenizer.h"

// Binary Number Tokenizer
class BinaryTokenizer : public Tokenizer
{
  public:
    BinaryTokenizer(std::string str)
    {
      // tokenize string for binary number
      // assumes valid input
      for (unsigned int i = 0; i < str.length(); i++)
        if (str[i] == '0')
          emplace_back('0',"0",1,0,0); // lineno and columno not applicable 
        else if (str[i] == '1')
          emplace_back('1',"1",1,0,0); // fill with default value 0
        else
          break;
    }
};

// Binary Number Parser
int main()
{
  // declare nonterminals and flow variables
  Parser<int> REC_NUM, IT_NUM, BIT, GETBIT;
  int x = 0, y = 0, b = 0, z = 0;

  // AFG
  // tail recursive starting nonterminal, store decimal result in z
  REC_NUM(x)>>z = GETBIT(x)>>y & REC_NUM(y)>>z 
    | GETBIT(x)>>z;

  // iterative starting nonterminal, store decimal result in z
  IT_NUM>>z = [&]{ z = 0; } & +( BIT>>b & [&]{ z = 2 * z + b; } );

  // match a bit and update the value of the decimal number result, z
  GETBIT(x)>>z = BIT>>b & [&]{ z = 2 * x + b; };
 
  // nonterminal to match a bit in the binary number
  // store semantic value in out-flow variable b
  BIT>>b = Token('0') & [&]{ b = 0; }
	  | Token('1') & [&]{ b = 1; };

  // get input
  std::cout << "Enter a binary number to convert: ";
  std::string input;
  std::cin >> input;

  // use the FlexTokenizer class to tokenize input string
  BinaryTokenizer tokens(input.c_str());

  // begin parsing
  // parse with REC_NUM
  if (REC_NUM.parse(&tokens))
  {
    // out-flow variable of REC_NUM is z
    std::cout << "REC_NUM Success! " << input << " is " << z << std::endl;
  }

  // parse with IT_NUM
  if (IT_NUM.parse(&tokens))
  {
    // out-flow variable of IT_NUM is z
    std::cout << "IT_NUM Success! " << input << " is " << z << std::endl;
  }

  return 0;
}
