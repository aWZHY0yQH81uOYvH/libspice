/*
	AST node for a comment so comments can be passed through to C++ code
*/

#pragma once

#include "Parser/ASTNode.hpp"

namespace spice {
namespace parser {

class Comment: public ASTNode {
protected:
	std::string text;
	
public:
	Comment(ASTNode *parent, NodePos pos, std::vector<std::string> &tokens);
	
	static const char * const regex_str;
	static const int regex_flags;
	static const size_t regex_groups;
	
	virtual std::string to_cpp() const;
};

}
}
