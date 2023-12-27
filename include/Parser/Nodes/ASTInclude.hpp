/*
 	AST node for a .include directive
*/

#pragma once

#include "Parser/ASTNode.hpp"

#include <string>

namespace spice {
namespace parser {

class ASTInclude: public ASTNode {
protected:
	std::string file;
	
public:
	ASTInclude(ASTNode *parent, NodePos pos, std::vector<std::string> &tokens);
	
	static const char * const regex_str;
	static const int regex_flags;
	static const size_t regex_groups;
	
	virtual std::string to_hpp() const;
	
	const std::string &get_file() const;
};

}
}

