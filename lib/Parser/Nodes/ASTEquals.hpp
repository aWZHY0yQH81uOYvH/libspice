/*
	AST node for an equals sign
*/

#pragma once

#include "Parser/ASTNode.hpp"

namespace spice {
namespace parser {

class ASTEquals: public ASTNode {
public:
	using ASTNode::ASTNode;
	
	static const char * const regex_str;
	static const int regex_flags;
	static const size_t regex_groups;
	
	virtual void to_cpp(FileInfo &fi) const override;
};

}
}
