/*
	AST node for a numeric literal
*/

#pragma once

#include "Parser/ASTNode.hpp"

namespace spice {
namespace parser {

class ASTNumericLiteral: public ASTNode {
protected:
	double number;
	
public:
	ASTNumericLiteral(ASTNode *parent, NodePos pos, std::vector<std::string> &tokens);
	
	static const char * const regex_str;
	static const int regex_flags;
	static const size_t regex_groups;
	
	virtual std::string to_cpp() const;
	
	double get_number() const;
};

}
}
