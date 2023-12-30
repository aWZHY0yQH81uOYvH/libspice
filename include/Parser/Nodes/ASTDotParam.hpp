/*
	AST node for a .param directive
*/

#pragma once

#include "Parser/ASTNode.hpp"

namespace spice {
namespace parser {

class ASTDotParam: public ASTNode {
private:
	enum MatchState {
		WAIT_FOR_IDENTIFIER,
		WAIT_FOR_EQUALS
	};
	
	MatchState match_state = WAIT_FOR_IDENTIFIER;
	
public:
	using ASTNode::ASTNode;
	
	static const char * const regex_str;
	static const int regex_flags;
	static const size_t regex_groups;
	
	virtual void verify() const override;
	
	virtual std::string all_to_cpp() const override;
	virtual std::string all_to_hpp() const override;
	
	virtual ASTNode *consume(ASTNode *&current_node, NodePos &current_pos, const char *&syntax, bool new_line) override;
	
	virtual bool exit_on_newline() const override;
};

}
}

