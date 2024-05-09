/*
	AST node for a numeric literal
*/

#pragma once

#include "Parser/Nodes/ASTExpression.hpp"

namespace spice {
namespace parser {

class ASTNumericLiteral: public ASTExpression {
protected:
	double number;
	
public:
	ASTNumericLiteral(NodePos pos, std::vector<std::string> &tokens);
	
	static const char * const regex_str;
	static const int regex_flags;
	static const size_t regex_groups;
	
	void verify() const override;
	
	virtual void to_cpp(FileInfo &fi) const override;
	
	double get_number() const;
};

}
}
