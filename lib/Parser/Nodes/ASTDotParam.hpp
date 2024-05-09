/*
	AST node for a .param directive
	https://ltwiki.org/files/LTspiceHelp.chm/html/DotParam.htm
*/

#pragma once

#include "Parser/ASTNode.hpp"

namespace spice {
namespace parser {

class ASTDotParam: public ASTNode {
private:
	std::vector<std::shared_ptr<ASTNode>*> identifiers;
	std::vector<std::shared_ptr<ASTNode>*> expressions;
	
public:
	using ASTNode::ASTNode;
	
	static const char * const regex_str;
	static const int regex_flags;
	static const size_t regex_groups;
	
	virtual void verify() const override;
	
	virtual void all_to_cpp(FileInfo &fi) const override;
	virtual void all_to_hpp(FileInfo &fi) const override;
	
	virtual ASTNode *consume(ASTNode *&current_node, NodePos &current_pos, const char *&syntax, bool new_line) override;
	
	virtual bool exit_on_newline() const override;
};

}
}
