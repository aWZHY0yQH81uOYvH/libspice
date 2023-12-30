/*
 	AST node for a .include directive
	https://ltwiki.org/files/LTspiceHelp.chm/html/DotInclude.htm
	(doesn't support fetching over HTTP)
*/

#pragma once

#include "Parser/ASTNode.hpp"

#include <string>

namespace spice {
namespace parser {

class ASTDotInclude: public ASTNode {
protected:
	std::string file;
	
public:
	ASTDotInclude(NodePos pos, std::vector<std::string> &tokens);
	
	static const char * const regex_str;
	static const int regex_flags;
	static const size_t regex_groups;
	
	virtual std::string to_hpp() const override;
	
	const std::string &get_file() const;
};

}
}

