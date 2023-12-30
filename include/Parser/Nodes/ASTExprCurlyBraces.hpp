/*
	AST node for a set of curly braces
*/

#pragma once

#include "Parser/ASTNode.hpp"
#include "Parser/Nodes/ASTExprParentheses.hpp"

namespace spice {
namespace parser {

class ASTExprCurlyBraces: public ASTExprParentheses {
public:
	ASTExprCurlyBraces(NodePos pos, std::vector<std::string> &tokens);
	
	static const char * const regex_str;
	static const int regex_flags;
	static const size_t regex_groups;
	
	virtual void verify() const override;
	
	virtual std::string all_to_cpp() const override;
};

}
}
