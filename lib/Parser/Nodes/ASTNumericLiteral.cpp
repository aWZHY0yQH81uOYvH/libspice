#include "Parser/Nodes/ASTNumericLiteral.hpp"
#include "Parser/Parser.hpp"

#include <unordered_map>
#include <cstring>
#include <cassert>
#include <regex.h>

namespace spice {
namespace parser {

// Any numeric digits, optional decimal, and optional leading digits, followed by an optional SI prefix or exponent format value
// Also consumes any following letters (like if you write 1Kohm)
const char * const ASTNumericLiteral::regex_str    = "^((\\+|-)?[0-9]*\\.?[0-9]+(e-?[0-9]+)?)(t|terra|g|giga|meg|k|kilo|m|milli|u|micro|n|nano|p|pico|f|femto)?[a-z]*";
const int          ASTNumericLiteral::regex_flags  = REG_ICASE;
const size_t       ASTNumericLiteral::regex_groups = 4;

ASTNumericLiteral::ASTNumericLiteral(NodePos pos, std::vector<std::string> &tokens): ASTExpression(pos) {
	number = std::stod(tokens.at(0));
	
	static const std::unordered_map<std::string, double> prefix_lut{
		{"t"    , 1e12},
		{"terra", 1e12},
		{"g"    , 1e9},
		{"giga" , 1e9},
		{"meg"  , 1e6},
		{"k"    , 1e3},
		{"kilo" , 1e3},
		{"m"    , 1e-3},
		{"milli", 1e-3},
		{"u"    , 1e-6},
		{"micro", 1e-6},
		{"n"    , 1e-9},
		{"nano" , 1e-9},
		{"p"    , 1e-12},
		{"pico" , 1e-12},
		{"f"    , 1e-15},
		{"femto", 1e-15}
	};
	
	std::string &prefix = tokens.at(3);
	if(prefix.length() > 0) {
		Parser::tolower(prefix);
		auto result = prefix_lut.find(prefix);
		assert(result != prefix_lut.end());
		number *= result->second;
	}
}

// Always valid once constructed
void ASTNumericLiteral::verify() const {}

void ASTNumericLiteral::to_cpp(FileInfo &fi) const {
	char buf[32];
	snprintf(buf, 32, "%e", number);
	*fi.out << buf;
}

double ASTNumericLiteral::get_number() const {
	return number;
}

}
}
