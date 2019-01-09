#ifndef PARSERPRINTER
#define PARSERPRINTER

#include <map>
#include <vector>
#include <string>
#include <iostream>

class ParserPrinter
{
  public:
    
    ParserPrinter()
    { }

    void name(const BaseParser * arg, std::string name)
    {
      assert(arg->tag_ == BaseParser::Tag::DEF || arg->tag_ ==
          BaseParser::Tag::NON || arg->tag_ == BaseParser::Tag::TOK);
      if (arg->tag_ == BaseParser::Tag::NON)
        names_[arg->get_def()] = name;
      else if (arg->tag_ == BaseParser::Tag::TOK)
        names_[arg->get_tok()] = name;
      else
        names_[arg] = name;
    }

    void print(const ParseTree * tree)
    {
      if (tree->has_parent())
        print_tree(tree,0);
    }

    std::string graphviz(const ParseTree * tree) 
    {
      std::string result;
      result = "strict graph {\n";
      result += print_graphviz(tree);
      result += "}";
      return result;
    } 

    void print(const BaseParser * arg, bool simple = false)
    {
      assert(arg->tag_ == BaseParser::Tag::DEF);
      
      if (names_[arg].empty())
        names_[arg] = generate_id(&arg, 65);
      std::cout << names_[arg];
      if (!simple && arg->get_in())
        std::cout << "(" << generate_id(arg->get_in(), 97) << ")";
      if (!simple && arg->get_out())
        std::cout << ">>" << generate_id(arg->get_out(), 97);
          
      std::cout << " = ";
       
      std::vector<BaseParser*> args;
      bool alt;
      if (arg->arg_.size() == 1 && arg->arg_[0]->tag_ == BaseParser::Tag::ALT)
      {
        args = arg->arg_[0]->arg_;
        alt = true;
      } else {
        args = arg->arg_;
        alt = false;
      }

      size_t num_spaces = names_[arg].size() + (!simple && arg->get_in() ? 5 : 0) + (!simple && arg->get_out() ? 5 : 0) + 1;
      for (auto a : args)
      {
        print_production(a, simple);
        if (alt && a != args.back())
        {
          std::cout << std::endl;
          for (size_t i = 0; i < num_spaces; i++)
            std::cout << " ";
          std::cout << "| ";
        }
      }
 
      std::cout << std::endl;
    }
     
  protected:
    
    void print_production(const BaseParser * arg, bool simple = false)
    {
      if (arg->tag_ == BaseParser::Tag::SEQ || arg->tag_ == BaseParser::Tag::ALT)
      {
        if (arg->min_ != 1 || arg->max_ != 1)
        {
          if (arg->max_ > 0)
            std::cout << "{ ";
          else
            std::cout << "( "; 
        }
      }
      switch(arg->tag_)
      {
        case BaseParser::Tag::ACT:
          std::cout << "? ACT ? ";
          break;
        case BaseParser::Tag::DEF:
          if (names_[arg].empty())
            names_[arg] = generate_id(&arg, 65);
          std::cout << names_[arg];

          std::cout << " ";
          break;
        case BaseParser::Tag::NON:
          if (names_[arg->get_def()].empty())
            names_[arg->get_def()] = generate_id(arg->get_def(), 65);
          std::cout << names_[arg->get_def()];
          
          if (!simple && arg->get_in())
            std::cout << "(" << generate_id(arg->get_in(), 97) << ")";
          if (!simple && arg->get_out())
            std::cout << ">>" << generate_id(arg->get_out(), 97);
          
          std::cout << " ";
          break;
        case BaseParser::Tag::TOK:
          if (!names_[arg->get_tok()].empty())
            std::cout << names_[arg->get_tok()];
          else if (!isprint(arg->get_tok_code()))
            std::cout << "(" << arg->get_tok_code() << ")";
          else
            std::cout << "'" << (char) arg->get_tok_code() << "'";

          if (!simple && arg->get_out())
            std::cout << ">>" << generate_id(arg->get_out(), 97);
          
          std::cout << " ";
          break;
        case BaseParser::Tag::SEQ:
          for (auto a : arg->arg_)
            print_production(a, simple);
          break;
        case BaseParser::Tag::ALT:
        {
          std::string s = (arg->min_ == 1 && arg->max_ == 1) ? "(" : "";
          for (auto a : arg->arg_)
          {
            std::cout << s;
            print_production(a, simple);
            s = "| ";
          }
          if (arg->min_ == 1 && arg->max_ == 1)
            std::cout << ") ";
          break;
        }
        default:
          break;
      }
      if (arg->tag_ == BaseParser::Tag::SEQ || arg->tag_ == BaseParser::Tag::ALT)
      {
        if (arg->max_ == 0 && arg->min_ > arg->max_)
          std::cout << ")~ ";
        else if (arg->max_ == 0)
          std::cout << ")! ";
        else if (arg->min_ == 0 && arg->max_ == 1)
          std::cout << "}{0,1} ";
        else if (arg->min_ == 1 && arg->max_ == BaseParser::MAX)
          std::cout << "}{1,} ";
        else if (arg->min_ == 0 && arg->max_ == BaseParser::MAX)
          std::cout << "} ";
        else if (arg->min_ > 1 && arg->min_ == arg->max_)
          std::cout << "}{" << arg->min_ << "} ";
        else if (arg->max_ > 1)
          std::cout << "}{" << arg->min_ << "," << arg->max_ << "} ";
      }
    }

    std::string print_graphviz(const ParseTree * tree)
    {
      std::string result;
      
      if (names_[tree->get_def()].empty())
        names_[tree->get_def()] = generate_id(tree->get_def(), 65); 
      result += "\t" + generate_id(tree, 65) + " [label=" + names_[tree->get_def()] + "];\n";

      if (tree->get_name()->empty())
      {
        result += "\t" + generate_id(tree, 65) + " -- { ";
        for (auto const &x : *tree->get_children())
          if (!x.get_name()->empty())
          {
            long temp_pointer = (long) x.get_name();
            std::string id_temp;
            id_temp = static_cast<char>(temp_pointer % 26 + 65);
            id_temp += static_cast<char>(temp_pointer / 10 % 26 + 65);
            id_temp += static_cast<char>(temp_pointer / 260 % 26 + 65);
            result += id_temp + " [label=" + "\"" + *x.get_name() + "\"] ";
          }
          else
          {
              result += generate_id(&x, 65) + " ";
          }
        result += "};\n";
      }
      for (auto const &x : *tree->get_children())
        if (x.get_name()->empty())
          result += print_graphviz(&x);
      return result;
    }

    void print_tree(const ParseTree * tree, int depth) const 
    {
      if (!tree->get_name()->empty())
      {
        std::cout << "\n";

        for (int x = 0; x < depth; x++)
          std::cout << "\t";
        std::cout << "{ ";
        std::cout << *tree->get_name();
        std::cout << " }";
        
        for (auto child : *tree->get_children())
          print_tree(&child, depth+1);
      }
      else if (tree->get_def())
      {
        std::cout << "\n";
        
        for (int x = 0; x < depth; x++)
          std::cout << "\t";
        std::cout << "{ ";
        
        if (names_.find(tree->get_def()) != names_.end())
          std::cout << names_.at(tree->get_def());
        else
          std::cout << generate_id(tree->get_def(), 65);
        
        for (auto child : *tree->get_children())
          print_tree(&child,depth+1);
        std::cout << "\n";
        
        for (int x = 0; x < depth; x++)
          std::cout << "\t";
        std::cout << "}";
      }
    }

    std::string generate_id(const void* nonterminal, int offset) const
    {
      std::string result;
      long temp_pointer = (long) nonterminal >> 2;
      result = static_cast<char>(temp_pointer % 26 + offset);
      result += static_cast<char>(temp_pointer / 26 % 26 + offset);
      result += static_cast<char>(temp_pointer / (26 * 26) % 26 + offset);
      return result;
    }

    std::map<const BaseParser*,std::string> names_;
};
#endif
