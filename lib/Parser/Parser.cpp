#include "Parser/Parser.hpp"
#include "Parser/ASTNode.hpp"

#include "Parser/Nodes/Comment.hpp"
#include "Parser/Nodes/Newline.hpp"

#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <utility>
#include <cctype>

namespace spice {
namespace parser {

// Find and load a file into the lines memory area
void Parser::load_file(const FileInfo &file) {
	// First find where the file is
	std::ifstream fs;
	
	for(auto &prefix:include_paths) {
		std::string path = prefix + "/" + file.path;
		fs.open(path);
		if(fs.is_open()) break;
	}
	
	if(!fs.is_open()) {
		std::ostringstream ss;
		
		ss << "Failed to find file " << file.path << std::endl;
		print_include_hierarchy(&file, ss);
		ss << std::endl;
		
		ss << "Looked in" << std::endl;
		for(auto &path:include_paths)
			ss << "\t" << path << std::endl;
		
		throw std::runtime_error(ss.str());
	}
	
	// Load lines into lines vector
	std::string line;
	while(std::getline(fs, line))
		lines.push_back(std::move(line));
	
	fs.close();
	
	current_file = &file;
}

// Convert SyntaxException into a runtime_exception with more information
void Parser::rethrow_syntax_error(const SyntaxException &se) const {
	std::ostringstream ss;
	
	ss << "Error in file " << current_file->path << " at line " << (se.pos.line + 1) << ", column " << (se.pos.character + 1) << std::endl;
	
	print_include_hierarchy(current_file, ss);
	
	ss << lines.at(se.pos.line) << std::endl;
	
	for(size_t x = 0; x < se.pos.character; x++)
		ss << " ";
	
	ss << "^" << std::endl;
	
	ss << se.error << std::endl;
	
	throw std::runtime_error(ss.str());
}

// Print paths leading to a file inclusion
void Parser::print_include_hierarchy(const FileInfo *file, std::ostringstream &ss) const {
	file = file->included_by;
	while(file) {
		ss << "\tIncluded by " << file->path << std::endl;
		file = file->included_by;
	}
}

// Run parsing on all files
void Parser::parse() {
	for(size_t file_ind = 0; file_ind < files.size(); file_ind++) {
		const FileInfo &fi = files[file_ind];
		
		load_file(fi);
		
		ASTNode *root_node = new ASTNode();
		ast_roots[file_ind].reset(root_node);
		
		ASTNode *current_node = root_node;
		
		for(size_t line_n = 0; line_n < lines.size(); line_n++) {
			const std::string &line = lines[line_n];
			
			NodePos np{line_n, 0};
			
			const char * const line_start = line.c_str();
			const char * const line_end = line_start + line.length();
			const char *line_ptr = line_start;
			
			bool new_line = true;
			
			while(line_ptr < line_end) {
				// Try to consume as much whitespace as possible
				if(std::isspace(*line_ptr)) {
					line_ptr++;
					continue;
				}
				
				// Try to consume as many comments as possible
				if(current_node->check_match<Comment>(np, line_ptr)) {
					// Add newline
					current_node->children.emplace_back(new Newline);
					continue;
				}
				
				// Handle line continuation + characters
				if(new_line && *line_ptr == '+') {
					// Add new line
					current_node->children.emplace_back(new Newline);
					line_ptr++;
					new_line = false;
					continue;
				}
				
				current_node->consume(current_node, np, line_ptr, new_line);
				new_line = false;
			}
		}
		
		// Verify
		try {
			root_node->all_verify();
		} catch(const SyntaxException &e) {
			rethrow_syntax_error(e);
		}
		
		// Make sure we got back to the root node
		if(current_node != root_node) {
			std::ostringstream ss;
			ss << "Unknown error in " << current_file->path << std::endl;
			print_include_hierarchy(current_file, ss);
			throw std::runtime_error(ss.str());
		}
	}
}

// Generate all C++ files
void Parser::gen_cpp(const std::string &prefix) const {
	// TODO
}

// Generate CMake file
void Parser::gen_cmake(const std::string &prefix) const {
	// TODO
	// Make a template thing in ../share?
}

void Parser::tolower(std::string &str) {
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

}
}
