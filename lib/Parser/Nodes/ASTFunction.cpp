#include "Parser/Nodes/ASTFunction.hpp"

#include <regex.h>

namespace spice {
namespace parser {

const char * const ASTFunction::regex_str    = "^([a-z_][a-z0-9_]*)[ \t]*\\(";
const int          ASTFunction::regex_flags  = REG_ICASE;
const size_t       ASTFunction::regex_groups = 1;

ASTFunction::ASTFunction(NodePos pos, std::vector<std::string> &tokens): ASTExpression(pos) {
	name = tokens.at(0);
}

std::string ASTFunction::all_to_cpp() const {
	// TODO
}

ASTNode *ASTFunction::consume(ASTNode *&current_node, NodePos &current_pos, const char *&syntax, bool new_line) {
	// TODO
}

std::string ASTFunction::get_name() const {
	return name;
}

}
}
