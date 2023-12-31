/*
	AST node that just adds a new line
*/

#pragma once

#include "Parser/ASTNode.hpp"

namespace spice {
namespace parser {

class ASTNewline: public ASTNode {
public:
	using ASTNode::ASTNode;
	ASTNewline() {}
	
	virtual void to_cpp(FileInfo &fi) const override {
		*fi.out << '\n';
		indent(fi);
	}
};

}
}
