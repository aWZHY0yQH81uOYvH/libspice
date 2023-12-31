/*
	Parser for processing SPICE files and converting them into C++ for use with libspice
*/

#pragma once

#include "Parser/ASTNode.hpp"

#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include <exception>
#include <filesystem>

namespace spice {
namespace parser {

class ASTNode;

class Parser {
protected:
	// List of files to process
	std::vector<std::unique_ptr<FileInfo>> files;
	
	// List of include paths to look for files in
	std::vector<std::filesystem::path> include_paths;
	
	// List of file lines
	std::vector<std::string> lines;
	
	// Currently loaded file
	const FileInfo *current_file = nullptr;
	
	// Find and load a file into the lines memory area
	void load_file(const FileInfo *file);
	
	// Print paths leading to a file inclusion
	void print_include_hierarchy(const FileInfo *start, std::ostringstream &ss) const;
	
	// Print code line followed by a line with a ^ pointing at the location specified by np
	void print_node_pos(const NodePos &np, std::ostringstream &ss) const;
	
public:
	// Root AST nodes for each file
	std::vector<std::unique_ptr<ASTNode>> ast_roots;
	
	// Add a file if it isn't already in the file list
	// Must be present in include_paths
	void add_file(std::filesystem::path path, const FileInfo *included_by = nullptr, const NodePos *include_pos = nullptr);
	
	// Add an include path; throws an expcetion if it doesn't exist
	void add_include_path(std::filesystem::path path);
	
	// Run parsing on all files
	void parse();
	
	// Generate output files
	// Return files generated
	std::vector<std::filesystem::path> gen_cpp(const std::filesystem::path &prefix) const;
	std::filesystem::path gen_cmake(const std::filesystem::path &prefix) const;
	
	// Utility functions
	static void tolower(std::string &str);
	static std::string quoted_path(const std::filesystem::path &p);
};

}
}
