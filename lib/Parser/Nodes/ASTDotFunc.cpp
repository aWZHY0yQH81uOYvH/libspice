#include "Parser/Nodes/ASTDotFunc.hpp"
#include "Parser/Nodes/ASTFunction.hpp"
#include "Parser/Nodes/ASTExprCurlyBraces.hpp"

#include <regex.h>

namespace spice {
namespace parser {

const char * const ASTDotFunc::regex_str    = "^\\.func[ \t]";
const int          ASTDotFunc::regex_flags  = REG_ICASE;
const size_t       ASTDotFunc::regex_groups = 0;

void ASTDotFunc::verify() const {
	if(!func)
		throw SyntaxException(pos, "Expected function declaration");
	
	if(!expr)
		throw SyntaxException((*func)->pos, "Expected function definition");
}

void ASTDotFunc::all_to_cpp(FileInfo &fi) const {
	for(auto &child:children) {
		// Create return statement before the expression
		if(child == *expr) {
			indent(fi);
			*fi.out << "return ";
			fi.indent_level++;
			child->children_to_cpp(fi);
		}
		
		else child->all_to_cpp(fi);
		
		// Increase indent and create brackets after function definition is created
		if(child == *func) {
			*fi.out << " {\n";
			fi.indent_level++;
		}
	}
	
	*fi.out << ";\n}\n";
	fi.indent_level -= 2;
}

void ASTDotFunc::all_to_hpp(FileInfo &fi) const {
	ASTNode::all_to_hpp(fi);
	*fi.out << ";\n";
}

ASTNode *ASTDotFunc::consume(ASTNode *&current_node, NodePos &current_pos, const char *&syntax, bool new_line) {
	(void)new_line;
	
	if(!func) {
		ASTNode *func_match = check_match<ASTFunction<true>>(current_pos, syntax);
		if(func_match) {
			func = &add_child(func_match);
			current_node = func_match;
			return func_match;
		}
	} else if(!expr) {
		ASTNode *expr_match = check_match<ASTExprCurlyBraces>(current_pos, syntax);
		if(expr_match) {
			expr = &add_child(expr_match);
			current_node = expr_match;
			return expr_match;
		}
	}
	
	return nullptr;
}

bool ASTDotFunc::exit_on_newline() const {
	return true;
}

}
}
