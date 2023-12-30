#include "Parser/Nodes/ASTDotInclude.hpp"
#include "Parser/Parser.hpp"

#include <filesystem>

#include <regex.h>

namespace spice {
namespace parser {

const char * const ASTDotInclude::regex_str    = "^\\.inc(lude)?[ \t]\"?([^ \t\"]+)\"?$";
const int          ASTDotInclude::regex_flags  = REG_ICASE;
const size_t       ASTDotInclude::regex_groups = 2;

ASTDotInclude::ASTDotInclude(NodePos pos, std::vector<std::string> &tokens): ASTNode(pos) {
	file = tokens.at(1);
	
}

std::string ASTDotInclude::to_hpp() const {
	std::filesystem::path header_file{file};
	header_file.replace_extension("hpp");
	
	return "#include " + Parser::quoted_path(header_file.filename()) + "\n";
}

const std::string &ASTDotInclude::get_file() const {
	return file;
}

}
}
