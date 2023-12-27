/*
	Abstract syntax tree node base class
*/

#pragma once

#include "Parser/Parser.hpp"

#include <string>
#include <vector>
#include <memory>
#include <unordered_set>
#include <typeinfo>
#include <typeindex>
#include <functional>

namespace spice {
namespace parser {

class ASTNode {
protected:
	// Verify syntax validity of just this node; throw exception if not valid
	virtual void verify() const;
	
	// Throw a verify error
	void verify_fail(std::string error) const;
	
	// Include files necessary for this node
	virtual const std::unordered_set<std::string> &include_files() const;
	
	// Convert this node to C++ representation
	virtual std::string to_cpp() const;
	virtual std::string to_hpp() const;
	
	// Regex string that matches what this node can parse
	static const char * const regex_str;
	
	// Regex flags (always ORed with REG_EXTENDED)
	static const int regex_flags;
	
	// How many tokens (capture groups) this regex should generate
	static const size_t regex_groups;
	
private:
	// Core of check_match; removing redundancy that is not type-dependent
	ASTNode *check_match_core(NodePos &current_pos, const char *&syntax,
	                          const char *regex_str, int regex_flags, size_t regex_groups,
	                          std::type_index ti, std::function<ASTNode*(ASTNode*, NodePos, std::vector<std::string>&)> alloc);
	
public:
	ASTNode *parent;
	NodePos pos;
	std::vector<std::shared_ptr<ASTNode>> children;
	
	// Basic constructor for initializing parent
	ASTNode(ASTNode *parent = nullptr, NodePos pos = {0, 0});
	
	// Take in vector of tokens, each corresponding to a capture group in the regex_str
	ASTNode(ASTNode *parent, NodePos pos, std::vector<std::string> &tokens);
	
	virtual ~ASTNode() {}
	
	// Convert this node and all children to C++ syntax
	virtual std::string all_to_cpp() const;
	virtual std::string all_to_hpp() const;
	
	// Verify this node and all children; throw exception if not valid
	void all_verify() const;
	
	// Get all include files needed for the C++ representation
	void all_include_files(std::unordered_set<std::string> &includes) const;
	
	// Consume syntax, generate child nodes, and move up/down the syntax tree
	void consume(ASTNode *&current_node, NodePos &current_pos, const char *&syntax, bool new_line);
	
	// Construct, append, and return a new child node of the given type if its regex matches the syntax input
	// Also moves the syntax pointer forwards
	template<typename T> T *check_match(NodePos &current_pos, const char *&syntax) {
		return static_cast<T*>(check_match_core(current_pos, syntax,
		                                        T::regex_str, T::regex_flags, T::regex_groups,
		                                        std::type_index(typeid(T)),
		                                        [](ASTNode *parent, NodePos pos, std::vector<std::string> &tokens) {
			return new T{parent, pos, tokens};
		}));
	}
};

}
}
