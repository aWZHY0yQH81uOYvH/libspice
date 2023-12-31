/*
	AST node for a function call
	If definition is true, arguments can only match identifiers instead of expressions
*/

#pragma once

#include "Parser/ASTNode.hpp"
#include "Parser/Nodes/ASTExpression.hpp"

#include <regex.h>

namespace spice {
namespace parser {

template<bool definition>
class ASTFunction: public ASTExpression {
protected:
	std::string name;
	std::vector<std::shared_ptr<ASTNode>*> args;
	bool closed = false;
	bool expecting_argument = false;
	bool if_statement = false;
	
	void all_to_xpp(FileInfo &fi, bool include_namespace) const;
	
public:
	ASTFunction(NodePos pos, std::vector<std::string> &tokens);
	
	static const char * const regex_str;
	static const int regex_flags;
	static const size_t regex_groups;
	
	virtual void verify() const override;
	
	virtual void all_to_cpp(FileInfo &fi) const override;
	virtual void all_to_hpp(FileInfo &fi) const override;
	
	virtual ASTNode *consume(ASTNode *&current_node, NodePos &current_pos, const char *&syntax, bool new_line) override;
	
	std::string get_name() const;
};

// Define these in header file so they are defined for all versions of the template class
template<bool def> const char * const ASTFunction<def>::regex_str    = "^([a-z_][a-z0-9_]*)[ \t]*\\(";
template<bool def> const int          ASTFunction<def>::regex_flags  = REG_ICASE;
template<bool def> const size_t       ASTFunction<def>::regex_groups = 1;

}
}
