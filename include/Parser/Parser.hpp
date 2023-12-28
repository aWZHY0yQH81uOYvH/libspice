/*
	Parser for processing SPICE files and converting them into C++ for use with libspice
*/

#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include <exception>
#include <filesystem>

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
	FileInfo(std::filesystem::path path, const FileInfo *included_by = nullptr): path(path), included_by(included_by) {}
	std::filesystem::path path;
	const FileInfo *included_by;
};

class ASTNode;

class Parser {
protected:
	// List of files to process
	std::vector<const FileInfo> files;
	
	// List of file lines
	std::vector<std::string> lines;
	
	// Currently loaded file
	const FileInfo *current_file = nullptr;
	
	// Find and load a file into the lines memory area
	void load_file(const FileInfo &file);
	
	// Print paths leading to a file inclusion
	void print_include_hierarchy(const FileInfo *start, std::ostringstream &ss) const;
	
	// Print code line followed by a line with a ^ pointing at the location specified by np
	void print_node_pos(const NodePos &np, std::ostringstream &ss) const;
	
public:
	// Root AST nodes for each file
	std::vector<std::unique_ptr<ASTNode>> ast_roots;
	
	// List of include paths to look for files in
	std::vector<std::filesystem::path> include_paths;
	
	// Add a file if it isn't already in the file list
	// Must be present in include_paths
	void add_file(std::filesystem::path path, const FileInfo *included_by = nullptr);
	
	// Run parsing on all files
	void parse();
	
	// Generate output files
	void gen_cpp(const std::string &prefix) const;
	void gen_cmake(const std::string &prefix) const;
	
	// Utility functions
	static void tolower(std::string &str);
	static std::string quoted_path(const std::filesystem::path &p);
};

}
}
