#include "Parser/Nodes/ASTExprCurlyBraces.hpp"

namespace spice {
namespace parser {

const char * const ASTExprCurlyBraces::regex_str    = "^{";
const int          ASTExprCurlyBraces::regex_flags  = 0;
const size_t       ASTExprCurlyBraces::regex_groups = 0;

ASTExprCurlyBraces::ASTExprCurlyBraces(NodePos pos, std::vector<std::string> &tokens): ASTExprParentheses(pos, tokens) {
	closing_char = '}';
}

void ASTExprCurlyBraces::verify() const {
	if(!closed)
		throw SyntaxException(pos, "Missing matching curly brace");
}

std::string ASTExprCurlyBraces::all_to_cpp() const {
	return "(" + children_to_cpp() + ")";
}

}
}
