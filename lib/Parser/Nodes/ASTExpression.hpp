/*
	ASTExpression -- AST node that's the base class of all expression type objects
	ASTEmptyExpression -- AST node representing an empty expression to be created and entered when an expression is expected
*/

#pragma once

#include "Parser/ASTNode.hpp"

namespace spice {
namespace parser {

class ASTExpression: public ASTNode {
public:
	using ASTNode::ASTNode;
	
	virtual ASTNode *consume(ASTNode *&current_node, NodePos &current_pos, const char *&syntax, bool new_line) override;
};

class ASTEmptyExpression: public ASTExpression {
protected:
	ASTNode *check_match_unary(NodePos &current_pos, const char *&syntax);

public:
	using ASTExpression::ASTExpression;
	
	virtual void verify() const override;
	
	virtual ASTNode *consume(ASTNode *&current_node, NodePos &current_pos, const char *&syntax, bool new_line) override;
};

}
}
