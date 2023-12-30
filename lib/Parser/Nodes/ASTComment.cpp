#include "Parser/Nodes/ASTComment.hpp"

namespace spice {
namespace parser {

// Expects a * or ; followed by any text until end of line
const char * const ASTComment::regex_str    = "^[\\*;](.*)$";
const int          ASTComment::regex_flags  = 0;
const size_t       ASTComment::regex_groups = 1;

ASTComment::ASTComment(NodePos pos, std::vector<std::string> &tokens): ASTNode(pos) {
	text = tokens.at(0);
}

std::string ASTComment::to_cpp() const {
	return "//" + text;
}

}
}