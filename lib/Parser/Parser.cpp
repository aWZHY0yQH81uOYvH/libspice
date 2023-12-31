#include "Parser/Parser.hpp"

#include "Parser/Nodes/ASTNewline.hpp"
#include "Parser/Nodes/ASTComment.hpp"
#include "Parser/Nodes/ASTDotInclude.hpp"

#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <utility>
#include <functional>
#include <cctype>

namespace spice {
namespace parser {

// Add a file if it isn't already in the file list
// Must be present in include_paths
void Parser::add_file(std::filesystem::path path, const FileInfo *included_by, const NodePos *include_pos) {
	
	// Look in search path if given a relative path
	if(path.is_relative()) {
		bool found = false;
		
		// Check in same directory as file it was included by
		// Temporarily add parent path of included file to include_paths
		bool pop_added_path = false;
		if(included_by) {
			auto canonical_path = std::filesystem::canonical(included_by->path.parent_path());
			if(std::find(include_paths.begin(), include_paths.end(), canonical_path) == include_paths.end()) {
				include_paths.push_back(canonical_path);
				pop_added_path = true;
			}
		}
		
		for(auto &search:include_paths) {
			auto p = search / path;
			
			// Check for existance
			if(!(std::filesystem::exists(p) && std::filesystem::is_regular_file(p)))
				continue;
			
			found = true;
			
			// Make canonical (resolve symlinks, make absolute)
			path = std::filesystem::canonical(p);
			break;
		}
		
		if(!found) {
			std::ostringstream ss;
			
			ss << "Failed to find file " << path << std::endl;
			if(include_pos)
				print_node_pos(*include_pos, ss);
			print_include_hierarchy(included_by, ss);
			ss << std::endl;
			
			ss << "Looked in" << std::endl;
			for(auto &p:include_paths)
				ss << "\t" << p << std::endl;
			
			throw std::runtime_error(ss.str());
		}
		
		// Remove temporary path if one was added
		if(pop_added_path)
			include_paths.pop_back();
	}
	
	// Check for uniqueness
	for(auto &fi:files)
		if(fi->path == path)
			return;
	
	// Add
	files.emplace_back(new FileInfo{path, included_by});
}

// Add an include path; if it does not exist, do nothing
void Parser::add_include_path(std::filesystem::path path) {
	path = std::filesystem::canonical(path);
	
	if(std::find(include_paths.begin(), include_paths.end(), path) == include_paths.end())
		include_paths.push_back(path);
}

// Find and load a file into the lines memory area
void Parser::load_file(const FileInfo *file) {
	current_file = file;
	
	std::ifstream fs(file->path);
	
	if(!fs.is_open()) {
		std::ostringstream ss;
		ss << "Unable to open file " << file->path << std::endl;
		print_include_hierarchy(file->included_by, ss);
		throw std::runtime_error(ss.str());
	}
	
	// Load lines into lines vector
	std::string line;
	lines.clear();
	while(std::getline(fs, line))
		lines.push_back(std::move(line));
	
	fs.close();
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
	std::ostringstream errors;
	
	ast_roots.resize(files.size());
	
	for(size_t file_ind = 0; file_ind < files.size(); file_ind++) {
		const FileInfo *fi = files[file_ind].get();
		
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
				np.character = line_ptr - line_start;
				
				// Try to consume as much whitespace as possible
				if(std::isspace(*line_ptr)) {
					line_ptr++;
					continue;
				}
				
				// Try to consume as many comments as possible
				if(*line_ptr == ';' || (*line_ptr == '*' && new_line)) {
					auto *comment = current_node->check_match<ASTComment>(np, line_ptr);
					if(comment) {
						current_node->add_child(comment);
						current_node->add_child(new ASTNewline); // Include newline
						continue;
					}
				}
				
				// Handle line continuation + characters
				if(new_line && *line_ptr == '+') {
					// Add newline
					current_node->add_child(new ASTNewline);
					line_ptr++;
					new_line = false;
					continue;
				}
				
				// Check if we are on a new line and one of the parent nodes requires exiting
				if(new_line) {
					ASTNode *node_ptr = current_node;
					ASTNode *highest_exit_node = nullptr;
					
					while(node_ptr) {
						if(node_ptr->exit_on_newline())
							highest_exit_node = node_ptr;
						node_ptr = node_ptr->parent;
					}
					
					if(highest_exit_node)
						current_node = highest_exit_node->parent;
				}
				
				// Attempt to consume while going up the node tree
				ASTNode *inserted_node = nullptr;
				while(current_node && !inserted_node) {
					inserted_node = current_node->consume(current_node, np, line_ptr, new_line);
					
					if(!inserted_node)
						current_node = current_node->parent;
				}
				
				// Make sure a new node was actually inserted
				if(!inserted_node) {
					errors << "Invalid syntax in file " << current_file->path << std::endl;
					print_include_hierarchy(current_file->included_by, errors);
					print_node_pos(np, errors);
					errors << std::endl;
					break;
				}
				
				// If we just added a .include directive, process that file too if we haven't already
				ASTDotInclude *asti = dynamic_cast<ASTDotInclude*>(inserted_node);
				if(asti)
					add_file(asti->get_file(), current_file, &np);
				
				new_line = false;
			}
		}
		
		// Verify
		std::vector<SyntaxException> error_list;
		root_node->all_verify(error_list);
		
		// Append to error list
		for(auto &error:error_list) {
			errors << "Error in file " << current_file->path << " at line " << (error.pos.line + 1) << ", column " << (error.pos.character + 1) << std::endl;
			
			print_include_hierarchy(current_file->included_by, errors);
			print_node_pos(error.pos, errors);
			
			errors << error.error << std::endl << std::endl;
		}
	}
	
	// Throw exception if any errors were generated
	if(errors.str().length())
		throw std::runtime_error(errors.str());
}

// Generate all C++ files
std::vector<std::filesystem::path> Parser::gen_cpp(const std::filesystem::path &prefix) const {
	std::vector<std::filesystem::path> generated_files;
	
	for(size_t file_ind = 0; file_ind < files.size(); file_ind++) {
		FileInfo *file = files[file_ind].get();
		const ASTNode *node = ast_roots[file_ind].get();
		
		std::filesystem::path cpp_path = prefix / file->path.filename();
		std::filesystem::path hpp_path = cpp_path;
		cpp_path.replace_extension("cpp");
		hpp_path.replace_extension("hpp");
		
		auto output = [&](const std::filesystem::path &p, const std::function<void(FileInfo&)> &generate) {
			std::ofstream out(p);
			if(!out.good())
				throw std::runtime_error("Can't open file for writing: " + quoted_path(p));
			
			file->indent_level = 0;
			file->out = &out;
			
			generate(*file);
			
			out.close();
			
			generated_files.push_back(p);
		};
		
		output(hpp_path, std::bind(&ASTNode::all_to_hpp, node, std::placeholders::_1));
		output(cpp_path, std::bind(&ASTNode::all_to_cpp, node, std::placeholders::_1));
	}
	
	return generated_files;
}

// Generate CMake file
std::filesystem::path Parser::gen_cmake(const std::filesystem::path &prefix) const {
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
