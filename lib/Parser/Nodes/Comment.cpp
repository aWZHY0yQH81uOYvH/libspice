#include "Parser/Nodes/Comment.hpp"

#include <cassert>

namespace spice {
namespace parser {

// Expects a * or ; followed by any text until end of line
const char * const Comment::regex_str    = "^[\\*;](.*)$";
const int          Comment::regex_flags  = 0;
const size_t       Comment::regex_groups = 1;

Comment::Comment(ASTNode *parent, NodePos pos, std::vector<std::string> &tokens): ASTNode(parent, pos) {
	text = tokens.at(0);
}

std::string Comment::to_cpp() const {
	return "//" + text;
}

}
}
