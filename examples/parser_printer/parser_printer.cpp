#include <iostream>
#include <fstream>
#include "parser.h"
#include "parsetree.h"
#include "parserprinter.h"
#include "flextokenizer.h"

// ParserPrinter Example
int main()
{
  /* specify nonterminals input and more
     specify terminals a, b, c, and d */
  Parser<> input, more, a('a'), b('b'), c('c'), d('d');

  size_t pos = 0;

  // AFG
  input = *(a) & ~(b | +(more));
  more = c & d;
  
  // create ParserPrinter object
  ParserPrinter  p;
 
  // assign names to nonterminals
  p.name(&input,"INPUT");
  p.name(&more,"MORE");

  // prints productions for nonterminal input and more to stdout
  p.print(&input,false);
  p.print(&more,false);

  // input string
  std::string text = "aaacdcdcd";

  // tokenize input string
  FlexTokenizer tokens(text);

  // create ParseTree object
  ParseTree tree;

  std::ofstream out("tree.dot");

  // begin parsing
  if (input.parse(&tokens,&pos,&tree))
  {
    std::cout << "Parsing successful!" << std::endl;
    
    // print parse tree to stdout
    p.print(&tree);

    out << p.graphviz(&tree);
  }

  out.close();

  return 0; 
}
