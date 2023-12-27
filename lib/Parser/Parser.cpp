#include "Parser/Parser.hpp"
#include "Parser/ASTNode.hpp"

#include "Parser/Nodes/ASTNewline.hpp"
#include "Parser/Nodes/ASTComment.hpp"
#include "Parser/Nodes/ASTInclude.hpp"

#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <utility>
#include <cctype>
#include <cassert>

namespace spice {
namespace parser {

// Add a file if it isn't already in the file list
// Must be present in include_paths
void Parser::add_file(std::filesystem::path path, const FileInfo *included_by) {
	
	// Look in search path if given a relative path
	if(path.is_relative()) {
		bool found = false;
		
		for(auto &search:include_paths) {
			auto p = search / path;
			
			// Resolve symlinks
			for(int link_count = 0; link_count < 100 && std::filesystem::is_symlink(p); link_count++)
				p = std::filesystem::read_symlink(p);
			
			if(std::filesystem::is_symlink(p))
				throw std::runtime_error("Too many symlinks for file " + quoted_path(path));
			
			// Check for existance
			if(std::filesystem::exists(p) && std::filesystem::is_regular_file(p)) {
				found = true;
				path = p;
				break;
			}
		}
		
		if(!found) {
			std::ostringstream ss;
			
			ss << "Failed to find file " << path << std::endl;
			print_include_hierarchy(included_by, ss);
			ss << std::endl;
			
			ss << "Looked in" << std::endl;
			for(auto &p:include_paths)
				ss << "\t" << p << std::endl;
			
			throw std::runtime_error(ss.str());
		}
	}
	
	// Check for uniqueness
	for(auto &fi:files)
		if(fi.path == path)
			return;
	
	// Add
	files.emplace_back(path, included_by);
}

// Find and load a file into the lines memory area
void Parser::load_file(const FileInfo &file) {
	current_file = &file;
	
	std::ifstream fs(file.path);
	
	if(!fs.is_open())
		throw std::runtime_error("Unable to open " + quoted_path(file.path));
	
	// Load lines into lines vector
	std::string line;
	while(std::getline(fs, line))
		lines.push_back(std::move(line));
	
	fs.close();
}

// Convert SyntaxException into a runtime_exception with more information
void Parser::rethrow_syntax_error(const SyntaxException &se) const {
	std::ostringstream ss;
	
	ss << "Error in file " << current_file->path << " at line " << (se.pos.line + 1) << ", column " << (se.pos.character + 1) << std::endl;
	
	print_include_hierarchy(current_file->included_by, ss);
	print_node_pos(se.pos, ss);
	
	ss << se.error << std::endl;
	
	throw std::runtime_error(ss.str());
}

// Print paths leading to a file inclusion
void Parser::print_include_hierarchy(const FileInfo *file, std::ostringstream &ss) const {
	while(file) {
		ss << "\tIncluded by " << file->path << std::endl;
		file = file->included_by;
	}
}

// Print code line followed by a line with a ^ pointing at the location specified by np
void Parser::print_node_pos(const NodePos &np, std::ostringstream &ss) const {
	ss << lines.at(np.line) << std::endl;
	
	for(size_t x = 0; x < np.character; x++)
		ss << " ";
	
	ss << "^" << std::endl;
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
				if(current_node->check_match<ASTComment>(np, line_ptr)) {
					// Add newline
					current_node->children.emplace_back(new ASTNewline);
					continue;
				}
				
				// Handle line continuation + characters
				if(new_line && *line_ptr == '+') {
					// Add new line
					current_node->children.emplace_back(new ASTNewline);
					line_ptr++;
					new_line = false;
					continue;
				}
				
				ASTNode *inserted_node = current_node->consume(current_node, np, line_ptr, new_line);
				
				// Make sure a new node was actually inserted
				if(!inserted_node) {
					std::ostringstream ss;
					ss << "Unknown token in " << current_file->path << std::endl;
					print_include_hierarchy(current_file->included_by, ss);
					print_node_pos(np, ss);
					throw std::runtime_error(ss.str());
				}
				
				// If we just added a .include directive, process that file too if we haven't already
				ASTInclude *asti = dynamic_cast<ASTInclude*>(inserted_node);
				if(asti)
					add_file(asti->get_file(), current_file);
				
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
			print_include_hierarchy(current_file->included_by, ss);
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

std::string Parser::quoted_path(const std::filesystem::path &p) {
	std::ostringstream ss;
	ss << p;
	return ss.str();
}

}
}
