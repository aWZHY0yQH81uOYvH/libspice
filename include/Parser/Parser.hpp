/*
	Parser for processing SPICE files and converting them into C++ for use with libspice
*/

#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include <exception>

namespace spice {
namespace parser {

// Information about where in a file a syntax node was from
struct NodePos {
	size_t line;
	size_t character;
};

// Exception thrown when syntax is wrong
class SyntaxException: public std::exception {
public:
	SyntaxException(NodePos pos, std::string error): pos(pos), error(error) {}
	
	NodePos pos;
	std::string error;
};

// Information about a file
struct FileInfo {
	FileInfo(std::string path, FileInfo *included_by = nullptr): path(path), included_by(included_by) {}
	std::string path;
	FileInfo *included_by;
};

class ASTNode;

class Parser {
protected:
	// List of file lines
	std::vector<std::string> lines;
	
	// Currently loaded file
	const FileInfo *current_file = nullptr;
	
	// Find and load a file into the lines memory area
	void load_file(const FileInfo &file);
	
	// Convert SyntaxException into a runtime_exception with more information
	void rethrow_syntax_error(const SyntaxException &se) const;
	
	// Print paths leading to a file inclusion
	void print_include_hierarchy(const FileInfo *file, std::ostringstream &ss) const;
	
public:
	// List of files to process
	std::vector<const FileInfo> files;
	
	// Root AST nodes for each file
	std::vector<std::unique_ptr<ASTNode>> ast_roots;
	
	// List of include paths to look for files in
	std::vector<std::string> include_paths;
	
	// Run parsing on all files
	void parse();
	
	// Generate output files
	void gen_cpp(const std::string &prefix) const;
	void gen_cmake(const std::string &prefix) const;
	
	// Utility functions
	static void tolower(std::string &str);
};

}
}
