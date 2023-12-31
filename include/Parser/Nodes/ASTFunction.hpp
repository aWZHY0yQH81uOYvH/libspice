/*
	AST node for a function call
*/

#pragma once

#include "Parser/ASTNode.hpp"
#include "Parser/Nodes/ASTExpression.hpp"

namespace spice {
namespace parser {

class ASTFunction: public ASTExpression {
protected:
	std::string name;
	
public:
	ASTFunction(NodePos pos, std::vector<std::string> &tokens);
	
	static const char * const regex_str;
	static const int regex_flags;
	static const size_t regex_groups;
	
	virtual void all_to_cpp(FileInfo &fi) const override;
	
	virtual ASTNode *consume(ASTNode *&current_node, NodePos &current_pos, const char *&syntax, bool new_line) override;
	
	std::string get_name() const;
};

}
}
