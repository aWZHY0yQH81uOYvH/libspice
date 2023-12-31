/*
	AST node for a set of parentheses
*/

#pragma once

#include "Parser/ASTNode.hpp"
#include "Parser/Nodes/ASTExpression.hpp"

namespace spice {
namespace parser {

class ASTExprParentheses: public ASTEmptyExpression {
protected:
	bool closed = false;
	char closing_char = ')';
	
public:
	using ASTEmptyExpression::ASTEmptyExpression;
	
	static const char * const regex_str;
	static const int regex_flags;
	static const size_t regex_groups;
	
	virtual void verify() const override;
	
	virtual void all_to_cpp(FileInfo &fi) const override;
	
	virtual ASTNode *consume(ASTNode *&current_node, NodePos &current_pos, const char *&syntax, bool new_line) override;
};

}
}
