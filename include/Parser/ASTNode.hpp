/*
	Abstract syntax tree node base class
*/

#pragma once

#include <string>
#include <vector>
#include <list>
#include <memory>
#include <unordered_set>
#include <typeinfo>
#include <typeindex>
#include <functional>
#include <filesystem>

// Commonly want to just enter a new node on a match in consume()
// Need ASTNode *node local variable
#define AST_CONSUME_ENTER(x) {node = check_match<x>(current_pos, syntax); if(node) {add_child(node); current_node = node; return node;}}

// Commonly want to go up node hierarchy
#define AST_CONSUME_GO_UP() {current_node = parent; return current_node ? current_node->consume(current_node, current_pos, syntax, new_line) : nullptr;}

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
	std::ostream *out = nullptr;
	size_t indent_level = 0;
};

class ASTNode {
public:
	ASTNode *parent = nullptr;
	NodePos pos;
	std::list<std::shared_ptr<ASTNode>> children;
	
	// Basic constructor
	ASTNode(NodePos pos = {0, 0});
	
	// Take in vector of tokens, each corresponding to a capture group in the regex_str
	ASTNode(NodePos pos, std::vector<std::string> &tokens);
	
	virtual ~ASTNode() {}
	
	
	// Regex string that matches what this node can parse
	static const char * const regex_str;
	
	// Regex flags (always ORed with REG_EXTENDED)
	static const int regex_flags;
	
	// How many tokens (capture groups) this regex should generate
	static const size_t regex_groups;
	
	
	// Convert this node to C++ representation
	virtual void to_cpp(FileInfo &fi) const;
	virtual void to_hpp(FileInfo &fi) const;
	
	// Convert all children to C++ syntax
	virtual void children_to_cpp(FileInfo &fi) const;
	virtual void children_to_hpp(FileInfo &fi) const;
	
	// Convert this node and all children to C++ syntax
	virtual void all_to_cpp(FileInfo &fi) const;
	virtual void all_to_hpp(FileInfo &fi) const;
	
	
	// Verify syntax validity of just this node; throw exception if not valid
	// Always call to ensure no errors before to_cpp functions
	virtual void verify() const;
	
	// Verify this node and all children; create list of errors
	void all_verify(std::vector<SyntaxException> &error_list) const;
	
	
	// Include files necessary for this node
	virtual const std::unordered_set<std::string> &include_files() const;
	
	// Get all include files needed for the C++ representation
	void all_include_files(std::unordered_set<std::string> &includes) const;
	
	
	// Consume syntax, generate child nodes, and move up/down the syntax tree
	virtual ASTNode *consume(ASTNode *&current_node, NodePos &current_pos, const char *&syntax, bool new_line);
	
	// Return true to force this node to exit on a newline
	virtual bool exit_on_newline() const;
	
	// Construct and return a new child node of the given type if its regex matches the syntax input
	// Also moves the syntax pointer forwards
	template<typename T> T *check_match(NodePos &current_pos, const char *&syntax) {
		return static_cast<T*>(check_match_core(current_pos, syntax,
		                                        T::regex_str, T::regex_flags, T::regex_groups,
		                                        std::type_index(typeid(T)),
		                                        [](NodePos pos, std::vector<std::string> &tokens) {
			return new T{pos, tokens};
		}));
	}
	
	// Get a reference to the shared_ptr child object pointing to the given pointer
	std::shared_ptr<ASTNode> &get_shared_ptr(ASTNode *to);
	
	// Add a child and update its parent
	std::shared_ptr<ASTNode> &add_child(ASTNode *child);
	std::shared_ptr<ASTNode> &add_child(std::shared_ptr<ASTNode> child);
	
private:
	// Core of check_match; removing redundancy that is not type-dependent
	ASTNode *check_match_core(NodePos &current_pos, const char *&syntax,
	                          const char *regex_str, int regex_flags, size_t regex_groups,
	                          std::type_index ti, std::function<ASTNode*(NodePos, std::vector<std::string>&)> alloc);
};

}
}
