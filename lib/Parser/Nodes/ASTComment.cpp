#include "Parser/Nodes/ASTComment.hpp"

namespace spice {
namespace parser {

// Expects a * or ; followed by any text until end of line
const char * const ASTComment::regex_str    = "^([\\*;])(.*)$";
const int          ASTComment::regex_flags  = 0;
const size_t       ASTComment::regex_groups = 2;

ASTComment::ASTComment(NodePos pos, std::vector<std::string> &tokens): ASTNode(pos) {
	start_of_line = tokens.at(0).at(0) == '*';
	text = tokens.at(1);
}

std::string ASTComment::to_cpp() const {
	std::string space;
	if(!start_of_line) space = " ";
	return space + "//" + text;
}

}
}
