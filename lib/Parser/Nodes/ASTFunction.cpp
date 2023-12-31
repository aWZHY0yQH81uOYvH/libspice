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

void ASTFunction::all_to_cpp(FileInfo &fi) const {
	// TODO
}

std::string ASTFunction::get_name() const {
	return name;
}

}
}
