/*
	AST node for an operator in an expression
*/

#pragma once

#include "Parser/Nodes/ASTExpression.hpp"

namespace spice {
namespace parser {

class ASTExprOperator: public ASTExpression {
public:
	enum OPType {
		OP_AND,
		OP_OR,
		OP_XOR,
		OP_GREATER,
		OP_LESS,
		OP_GREATER_EQUALS,
		OP_LESS_EQUALS,
		OP_ADD,
		OP_SUBTRACT,
		OP_MULTIPLY,
		OP_DIVIDE,
		OP_POWER,
		OP_MAX
	};
	
protected:
	OPType op_type;
	
	static const std::string op_lut[OP_MAX];
	
public:
	ASTExprOperator(NodePos pos, std::vector<std::string> &tokens);
	
	static const char * const regex_str;
	static const int regex_flags;
	static const size_t regex_groups;
	
	virtual void all_to_cpp(FileInfo &fi) const override;
	
	OPType get_op_type() const;
};

}
}
