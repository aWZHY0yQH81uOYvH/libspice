#include "Parser/Nodes/ASTEquals.hpp"

namespace spice {
namespace parser {

// Expects a * or ; followed by any text until end of line
const char * const ASTEquals::regex_str    = "^=";
const int          ASTEquals::regex_flags  = 0;
const size_t       ASTEquals::regex_groups = 0;

std::string ASTEquals::to_cpp() const {
	return " = ";
}

}
}
