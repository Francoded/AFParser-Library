# [Project Title]

[Project Title] is a collection of C++11 header files that provides users with a
flexible rapid prototyping tool to create general-purpose LL(k) parsers in C++. 
Attribute-Flow Grammars (AFG) are used to specify the LL(k) language the created
parser will accept. Unlike parser generators like Bison and YACC, [Project
Title] does not require installation or code generation.

## Getting Started

To illustrate how to use [Project Name] to create a parser in C++11, we will be
using a simplified version of the binary number parser program as our running 
example. The full program can be found in the Examples wiki page.

### Prerequisites

Compilers:
* GNU g++
* Clang

### General Usage

1) Declare (non)terminals
   * Nonterminal Syntax: Parser<InType, OutType> NAME;
   * Terminal Syntax: Parser<InType, OutType> term(token_code);
   * InType and OutType specify the types of the in- and out-flow variable for
  the (non)terminal
   * token_code is an integer

2) Specify language to accept with an AFG

3) Prepare input in a Tokenizer derived object
   * FlexTokenizer is derived from Tokenizer and uses Flex to tokenize input

4) Invoke parse() on starting nonterminal

The following is an example C++ program that uses [Project Name] to implement a 
parser that accepts the language of all binary numbers.

binary.cpp:
```C++
#include <iostream>

#include "parser.h" // defines Parser
#include "flextokenizer.h" // defines FlexTokenizer

int main()
{
  // 1. Declare (non)terminals
  Parser<> NUM, BIT;

  // 2. Attribute-Flow Grammar
  NUM = BIT & NUM
	  | BIT;

  BIT>>b = Token(‘0’)
	  | Token(‘1’);

  // 3. prepare input
  std::string text = “1101010”;
  FlexTokenizer tokens(text); // FlexTokenizer is a derived
                              // class of Tokenizer

  // 4. parse() invoked on NUM
  if (NUM.parse(&tokens))
    std::cout << "Accepted: " << text << std::endl;

  return 0;
}
```

### Attribute-Flow Grammars (AFG)

The syntax of an Attribute-Flow Grammar is similar to that of grammars written
in Extended-Backus Naur Form (EBNF).

Key Differences:
1) Grammar productions are specfied with an equal sign (=) instead of an arrow
(->) and end with a semicolon (;).
   * EBNF: S -> A
   * AFG: S = A;
2) Sequential grammar symbols are separated by the binary AND operator (&)
   * EBNF: S -> A B
   * AFG: S = A & B;
3) Grammar symbol operators appear on the left of the grammar symbol operand as opposed to EBNF notation where the operator appears to the right of the grammar symbol operand
   * EBNF: S -> A\*;
   * AFG: S = \*A;

The following table illustrates all the operations available to be performed on
grammar symbols in an AFG.
```
X & Y                concatenation
X | Y                alternation
*X                   repeat
+X                   nonzero repeat
-X                   optional
~X                   lookahead (match and backtrack)
!X                   negative lookahead (non-match and backtrack)
N * X                repeat N times
N-M * X              repeat N to M times
[&]{ ... }           semantic action
Token('A')           a token with code 65 (ASCII value of 'A')
Token(65)            a token with code 65
```

### Preparing Input

The created LL(k) parser expects token-based input stored in a Tokenizer-derived
object. FlexTokenizer is a derived Tokenizer class that uses Flex generated scanner
to tokenize input.

The following is the Flex specification used to generate the Flex lexer for the
binary number parser example given earlier.

lexer.l:
```
%{
%}

%option noyywrap

%%
[01]          { return *yytext; }
.             { /* do nothing */ }
%%
```

Use Flex to generate the lexer in lex.yy.c:

```
flex lexer.l
```

The generated lexer, lex.yy.c, will then be linked when compiling:

```
g++ -c lex.yy.c
g++ binary.cpp lex.yy.o
```

Flex documentation can be found at http://westes.github.io/flex/manual/. 
Implementing Tokenizer derived classes are discussed more in the Scanning wiki page.

### Semantics

In AFGs, flow variables give grammar symbols a semantic meaning. Each grammar 
symbol may have an in- and out-flow variable which replace inherited and
synthesize attributes, used in conventional attribute grammars, respectively.

AFGs use C++ lambdas to implement semantic actions in grammar productions.

AFG semantics is discussed more in the Attribute Flow Grammars wiki page.

### Visualization

One can visualize grammar productions or parse trees for some input using the
ParseTree and PrettyParser classes.

The PrettyParser class can visualize:
* Grammar productions to stdout
* Parse trees, for some input, to stdout and Graphviz Dot notation

This is discussed further in the Visualization wiki page.

### Examples

There are numerous examples to reference and are provided in the examples folder of the repository. The examples are discussed further in the Examples wiki section.
