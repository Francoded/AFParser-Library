#include <cstring>
#include <set>
#include "parser.h"
#include "tokenizer.h"
#include "tokenstream.h"

#define WORD_TOK_CODE -1

// sentenceTokenizer tokenizes words based on spaces
class sentenceTokenizer : public Tokenizer
{
  public:
  sentenceTokenizer(std::string str)
  {
    // make copy of input string
    char * input = nullptr;
    input = strdup(str.c_str());

    // get token
    char * tok = std::strtok(input," ");
    
    // check if token was found
    while (tok != NULL)
    {
      // token is a word, add token
      emplace_back(WORD_TOK_CODE,tok,strlen(tok),0,0);

      // get next token
      tok = std::strtok(NULL,", ");
    }
    free(input);
  }
};

// Overload extraction operator of TokenStream class to allow for 
// word type checking
TokenStream<std::set<std::string>>&
    operator>>(TokenStream<std::set<std::string>> & in, int& out)
{
  (void) out;

  // Used to not accept the following edge cases:
  // "I [verb] me" and "We [verb] us" and "You [verb] you"
  if (in.get_text() == "I" || in.get_text() == "me")
    out = 1;
  else if (in.get_text() == "We" || in.get_text() == "us")
    out = 2;
  else if (in.get_text() == "You" || in.get_text() == "you")
    out = 4;
  else
    out = 0;

  // Check if word is of correct type
  if (in.get_in()->find(in.get_text()) == in.get_in()->end())
    throw extraction_error("Invalid word");
   
  return in;
}

int main()
{
  int subject_flag = 0, object_flag = 0, dummy = 0;

  // sets for different types of words
  std::set<std::string> mixed_subject_words, third_sing_subj_words;
  std::set<std::string> object_words;
  std::set<std::string> verb_words1, verb_words2;

  // add third singular subject words
  third_sing_subj_words.insert("He");
  third_sing_subj_words.insert("She");
  third_sing_subj_words.insert("It");

  // mixed subject words include 1st and 2nd tense 
  // (singular and plural) and 3rd tense plural
  mixed_subject_words.insert("I");
  mixed_subject_words.insert("We");
  mixed_subject_words.insert("You");
  mixed_subject_words.insert("They");

  // add object words
  object_words.insert("me");
  object_words.insert("us");
  object_words.insert("you");
  object_words.insert("him");
  object_words.insert("her");
  object_words.insert("it");
  object_words.insert("them");

  // add verbs
  verb_words1.insert("like");
  verb_words1.insert("hate");
  verb_words1.insert("love");

  // add verbs
  verb_words2.insert("likes");
  verb_words2.insert("hates");
  verb_words2.insert("loves");

  // declare (non)terminals
  Parser<> sentence, verb_phrase;
  Parser<std::set<std::string>,int> word(-1);

  // AFG
  sentence = ( 
      word(mixed_subject_words)>>subject_flag & word(verb_words1)>>dummy 
    | word(third_sing_subj_words)>>subject_flag & word(verb_words2)>>dummy
    ) & word(object_words)>>object_flag 
      & [&]{ if(subject_flag & object_flag){ throw parsing_error(""); } };

  // begin user prompt
  std::cout << "==============================================\n\n";
  std::cout << "\tA simple present-tense pronoun sentence parser...\n\n";
  std::cout << "\tVerbs accepted: Love, Hate, Like
  std::cout << "\tFormat: subject_pronoun verb object_pronoun \n\n";
  std::cout << "==============================================\n\n";

  std::cout << "Give me a sentence.\n";
  std::cout << "Type [q or Q] to quit: ";

  std::string input;
  while (getline(std::cin, input))
  {
    // quit 
    if (input.empty() || input[0] == 'q' || input[0] == 'Q')
      break;

    // tokenize input string
    sentenceTokenizer tokens(input);

    // parse using sentence as starting nonterminal
    if (sentence.parse(&tokens))
    {
      std::cout << "-------------------------\n";
      std::cout << "Parsing succeeded\n";
      std::cout << input << " parses OK.\n" << std::endl;
    }
    else
    {
      std::cout << "-------------------------\n";
      std::cout << "Parsing failed\n" << std::endl;
    }

    // prompt user
    std::cout << "Give me a sentence.\n";
    std::cout << "Type [q or Q] to quit: ";

  }

  std::cout << "\nBye." << std::endl;
  return 0;
}
