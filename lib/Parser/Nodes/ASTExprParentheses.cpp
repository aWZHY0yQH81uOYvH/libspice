#include "Parser/Nodes/ASTExprParentheses.hpp"

namespace spice {
namespace parser {

const char * const ASTExprParentheses::regex_str    = "^\\(";
const int          ASTExprParentheses::regex_flags  = 0;
const size_t       ASTExprParentheses::regex_groups = 0;

void ASTExprParentheses::verify() const {
	if(!closed)
		throw SyntaxException(pos, "Missing matching parenthesis");
}

std::string ASTExprParentheses::all_to_cpp() const {
	return "(" + children_to_cpp() + ")";
}

ASTNode *ASTExprParentheses::consume(ASTNode *&current_node, NodePos &current_pos, const char *&syntax, bool new_line) {
	(void)new_line;
	
	// Mark as closed if we see the closing character
	// (variable to reduce code duplication when doing curly braces)
	if(*syntax == closing_char) {
		syntax++;
		closed = true;
		return this;
	}
	
	if(!closed) {
		// Use standard empty expression processing except don't allow replacing ourselves because we need to keep the information that parentheses are present
		ASTNode *node = check_match_unary(current_pos, syntax);
		if(node) {
			add_child(node);
			current_node = node;
		}
		return node;
	}
	
	// Turn back into a normal expression when we're closed
	else return ASTExpression::consume(current_node, current_pos, syntax, new_line);
}

}
}
