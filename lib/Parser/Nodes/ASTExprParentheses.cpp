#include "Parser/Nodes/ASTExprParentheses.hpp"

namespace spice {
namespace parser {

const char * const ASTExprParentheses::regex_str    = "^\\(";
const int          ASTExprParentheses::regex_flags  = 0;
const size_t       ASTExprParentheses::regex_groups = 0;

bool ASTExprParentheses::have_content() const {
	for(auto &child:children)
		if(dynamic_cast<ASTExpression*>(child.get()))
			return true;
	return false;
}

void ASTExprParentheses::verify() const {
	if(!closed)
		throw SyntaxException(pos, "Missing matching parenthesis");
	
	if(!have_content())
		throw SyntaxException(pos, "Expected expression");
}

void ASTExprParentheses::all_to_cpp(FileInfo &fi) const {
	*fi.out << '(';
	children_to_cpp(fi);
	*fi.out << ')';
}

ASTNode *ASTExprParentheses::consume(ASTNode *&current_node, NodePos &current_pos, const char *&syntax, bool new_line) {
	(void)new_line;
	
	// Mark as closed if we see the closing character
	// (variable to reduce code duplication when doing curly braces)
	if(!closed && *syntax == closing_char) {
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
