#include "Parser/Nodes/ASTInclude.hpp"
#include "Parser/Parser.hpp"

#include <filesystem>

#include <regex.h>

namespace spice {
namespace parser {

const char * const ASTInclude::regex_str    = "^\\.include \"?([^ \t]+)\"?$";
const int          ASTInclude::regex_flags  = REG_ICASE;
const size_t       ASTInclude::regex_groups = 1;

ASTInclude::ASTInclude(ASTNode *parent, NodePos pos, std::vector<std::string> &tokens): ASTNode(parent, pos) {
	file = tokens.at(0);
	
}

std::string ASTInclude::to_hpp() const {
	std::filesystem::path header_file{file};
	header_file.replace_extension("hpp");
	
	return "#include " + Parser::quoted_path(header_file) + "\n";
}

const std::string &ASTInclude::get_file() const {
	return file;
}

}
}
