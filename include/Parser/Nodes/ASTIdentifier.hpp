/*
	AST node for a variable identifier
*/

#pragma once

#include "Parser/Nodes/ASTExpression.hpp"

namespace spice {
namespace parser {

class ASTIdentifier: public ASTExpression {
protected:
	std::string name;
	
public:
	ASTIdentifier(NodePos pos, std::vector<std::string> &tokens);
	
	static const char * const regex_str;
	static const int regex_flags;
	static const size_t regex_groups;
	
	virtual std::string to_cpp() const override;
	
	const std::string &get_name() const;
};

}
}

