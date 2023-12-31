#include "Parser/Nodes/ASTIdentifier.hpp"

#include <regex.h>

namespace spice {
namespace parser {

// Allow identifiers that work in C++
const char * const ASTIdentifier::regex_str    = "^([a-z_][a-z0-9_]*)";
const int          ASTIdentifier::regex_flags  = REG_ICASE;
const size_t       ASTIdentifier::regex_groups = 1;

ASTIdentifier::ASTIdentifier(NodePos pos, std::vector<std::string> &tokens): ASTExpression(pos) {
	name = tokens.at(0);
}

void ASTIdentifier::to_cpp(FileInfo &fi) const {
	*fi.out << name;
}

const std::string &ASTIdentifier::get_name() const {
	return name;
}

}
}
