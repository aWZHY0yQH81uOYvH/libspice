#include "Parser/Nodes/ASTDotParam.hpp"
#include "Parser/Nodes/ASTIdentifier.hpp"
#include "Parser/Nodes/ASTExpression.hpp"
#include "Parser/Nodes/ASTEquals.hpp"

#include <cassert>
#include <regex.h>

namespace spice {
namespace parser {

const char * const ASTDotParam::regex_str    = "^\\.params?[ \t]";
const int          ASTDotParam::regex_flags  = REG_ICASE;
const size_t       ASTDotParam::regex_groups = 0;

void ASTDotParam::verify() const {
	// Need at least one identifier
	if(identifiers.size() == 0)
		throw SyntaxException(pos, "Expected identifier");
	
	// Need an equal number of expressions and identifiers
	if(identifiers.size() != expressions.size())
		throw SyntaxException((**identifiers.rbegin())->pos, "Expected assignment to identifier");
}

void ASTDotParam::all_to_cpp(FileInfo &fi) const {
	// TODO: implement for real
	
	ASTNode::all_to_cpp(fi);
}

void ASTDotParam::all_to_hpp(FileInfo &fi) const {
	// TODO
}

ASTNode *ASTDotParam::consume(ASTNode *&current_node, NodePos &current_pos, const char *&syntax, bool new_line) {
	(void)new_line;
	
	// If have as many expressions as identifiers, we need another identifier
	if(identifiers.size() == expressions.size()) {
		auto *id = check_match<ASTIdentifier>(current_pos, syntax);
		if(id) {
			identifiers.push_back(&add_child(id));
			return id;
		}
	}
	
	// Otherwise we need an equals sign followed by an expression
	else {
		auto *equals = check_match<ASTEquals>(current_pos, syntax);
		if(equals) {
			auto *expr = new ASTEmptyExpression(current_pos);
			add_child(equals);
			expressions.push_back(&add_child(expr));
			current_node = expr;
			return expr;
		}
	}
	
	return nullptr;
}

bool ASTDotParam::exit_on_newline() const {
	return true;
}

}
}
