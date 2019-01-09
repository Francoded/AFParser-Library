#ifndef PARSETREE
#define PARSETREE

#include <vector>
#include <string>
#include <iostream>

// Forward Declare BaseParser class
class BaseParser;

class ParseTree
{
  public:
    ParseTree()
      :
        name_(""),
        def_(NULL)
    { }
    ParseTree(std::string name)
      :
        name_(name),
        def_(NULL)
    { }
    void set_parent(std::string name)
    {
      name_ = name;
      def_ = NULL;
    }
    void set_parent(const BaseParser* arg)
    {
      name_ = "";
      def_ = arg;
    }
    const std::string * get_name() const
    {
      return &name_;
    }
    const BaseParser* get_def() const
    {
      return def_;
    }
    bool has_parent() const
    {
      return def_ ? true : !name_.empty();
    }
    const std::vector<ParseTree>* get_children() const
    {
      return &children_;
    }
    ParseTree* add_child(ParseTree child)
    {
      children_.emplace_back(child);
      return &children_.back();
    }
    void clear()
    {
      *this = ParseTree();
    }
    void clear_children()
    {
      children_.clear();
    }
    void print_tree(int depth = 0) const
    {
      for (int i = 0; i < depth; i++)
        std::cout << "\t";
      if (!name_.empty())
        std::cout << "{ " << name_ << "\n";
      else if (def_)
        std::cout << "{ " << def_ << "\n";
      for (auto x : children_)
        x.print_tree(depth + 1);
      for (int i = 0; i < depth; i++)
        std::cout << "\t";
      std::cout << "}\n";
    }
  protected:
    std::string name_;
    const BaseParser* def_;
    std::vector<ParseTree> children_;
};
#endif
