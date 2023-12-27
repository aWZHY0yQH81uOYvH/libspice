/*
	AST node that just adds a new line
*/

#pragma once

#include "Parser/ASTNode.hpp"

namespace spice {
namespace parser {

class Newline: public ASTNode {
public:
	using ASTNode::ASTNode;
	Newline() {}
	
	virtual std::string to_cpp() const {
		return "\n";
	}
};

}
}
