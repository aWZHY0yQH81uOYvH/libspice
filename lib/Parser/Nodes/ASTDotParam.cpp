#include "Parser/Nodes/ASTDotParam.hpp"
#include "Parser/Nodes/ASTIdentifier.hpp"
#include "Parser/Nodes/ASTExpression.hpp"
#include "Parser/Nodes/ASTEquals.hpp"

#include <cassert>
#include <regex.h>

namespace spice {
namespace parser {

const char * const ASTDotParam::regex_str    = "^.params?[ \t]";
const int          ASTDotParam::regex_flags  = REG_ICASE;
const size_t       ASTDotParam::regex_groups = 0;

void ASTDotParam::verify() const {
	// If we're still waiting for an equals sign, we must be missing an expression
	if(match_state != WAIT_FOR_IDENTIFIER) {
		ASTNode *id = nullptr;
		for(auto child_it = children.rbegin(); child_it != children.rend(); child_it++) {
			auto *id_found = dynamic_cast<ASTIdentifier*>(child_it->get());
			if(id_found) id = id_found;
		}
		
		assert(id);
		
		throw SyntaxException(id->pos, "Expected assignment to identifier");
	}
}

std::string ASTDotParam::all_to_cpp() const {
	// TODO: implement for real
	
	return ASTNode::all_to_cpp();
}

std::string ASTDotParam::all_to_hpp() const {
	// TODO
}

ASTNode *ASTDotParam::consume(ASTNode *&current_node, NodePos &current_pos, const char *&syntax, bool new_line) {
	(void)new_line;
	
	switch(match_state) {
		case WAIT_FOR_IDENTIFIER: {
			// Consume an identifier then wait for an equals sign
			auto *id = check_match<ASTIdentifier>(current_pos, syntax);
			if(id) {
				add_child(id);
				match_state = WAIT_FOR_EQUALS;
				return id;
			}
			break;
		}
		
		case WAIT_FOR_EQUALS: {
			// Match an equals then enter an expression node
			auto *equals = check_match<ASTEquals>(current_pos, syntax);
			if(equals) {
				auto *expr = new ASTEmptyExpression(current_pos);
				add_child(equals);
				add_child(expr);
				current_node = expr;
				match_state = WAIT_FOR_IDENTIFIER;
				return expr;
			}
			break;
		}
			
		default:
			break;
	}
	
	return nullptr;
}

bool ASTDotParam::exit_on_newline() const {
	return true;
}

}
}
