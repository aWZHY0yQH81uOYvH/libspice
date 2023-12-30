#include "Parser/ASTNode.hpp"

#include "Parser/Nodes/ASTComment.hpp"
#include "Parser/Nodes/ASTNumericLiteral.hpp"
#include "Parser/Nodes/ASTDotInclude.hpp"
#include "Parser/Nodes/ASTDotParam.hpp"

#include <cassert>
#include <unordered_map>
#include <regex.h>

namespace spice {
namespace parser {

const char * const ASTNode::regex_str    = ".*";
const int          ASTNode::regex_flags  = 0;
const size_t       ASTNode::regex_groups = 0;

ASTNode::ASTNode(NodePos pos): pos(pos) {}

ASTNode::ASTNode(NodePos pos, std::vector<std::string> &tokens): ASTNode(pos) {
	(void)tokens;
}

// Convert this node to C++ representation
std::string ASTNode::to_cpp() const {
	return "";
}

std::string ASTNode::to_hpp() const {
	return "";
}

// Convert all children to C++ syntax
std::string ASTNode::children_to_cpp() const {
	std::string ret;
	for(auto &child:children)
		ret += child->all_to_cpp();
	return ret;
}

std::string ASTNode::children_to_hpp() const {
	std::string ret;
	for(auto &child:children)
		ret += child->all_to_hpp();
	return ret;
}

// Convert this node and all children to C++ syntax
std::string ASTNode::all_to_cpp() const {
	return to_cpp() + children_to_cpp();
}

std::string ASTNode::all_to_hpp() const {
	return to_hpp() + children_to_hpp();
}

// Verify syntax validity of just this node; throw exception if not valid
void ASTNode::verify() const {}

// Verify this node and all children
void ASTNode::all_verify(std::vector<SyntaxException> &error_list) const {
	try {
		verify();
	} catch(const SyntaxException &se) {
		error_list.push_back(se);
	}
	
	for(auto &child:children)
		child->all_verify(error_list);
}

// Include files necessary for this node
const std::unordered_set<std::string> &ASTNode::include_files() const {
	static const std::unordered_set<std::string> our_includes = {};
	return our_includes;
}

// Get all include files needed for the C++ representation
void ASTNode::all_include_files(std::unordered_set<std::string> &includes) const {
	const auto &our_includes = include_files();
	includes.insert(our_includes.begin(), our_includes.end());
	
	for(auto &child:children)
		child->all_include_files(includes);
}

// Core of check_match; removing redundancy that is not type-dependent
// Construct and return a new child node of the given type if its regex matches the syntax input
// Also moves the syntax pointer forwards
// Intended to only be called through check_match
ASTNode *ASTNode::check_match_core(NodePos &current_pos, const char *&syntax,
                                   const char *regex_str, int regex_flags, size_t regex_groups,
                                   std::type_index ti, std::function<ASTNode*(NodePos, std::vector<std::string>&)> alloc) {
	// Cache compiled regexes
	static std::unordered_map<std::type_index, std::unique_ptr<regex_t>> re_cache;
	
	auto &re = re_cache[ti];
	
	// Compile the regex if it's not in the cache
	if(!re) {
		re.reset(new regex_t);
		int ret = regcomp(re.get(), regex_str, regex_flags | REG_EXTENDED);
		assert(ret == 0);
	}
	
	// Attempt to match
	static std::vector<regmatch_t> matches;
	if(regex_groups + 1 > matches.size())
		matches.resize(regex_groups + 1);
	
	int ret = regexec(re.get(), syntax, matches.size(), matches.data(), 0);
	
	// Match failed
	if(ret) {
		assert(ret == REG_NOMATCH);
		return nullptr;
	}
	
	// Create strings from each group
	static std::vector<std::string> groups;
	if(regex_groups > groups.size())
		groups.resize(regex_groups);
	
	for(size_t ind = 0; ind < regex_groups; ind++) {
		const regmatch_t &match = matches[ind + 1];
		std::string &group = groups[ind];
		
		// Check if capture group didn't match anything
		if(match.rm_so < 0 || match.rm_eo < 0) {
			group.clear();
			continue;
		}
		
		// Generate a substring
		group = std::string(syntax + match.rm_so, match.rm_eo - match.rm_so);
	}
	
	// Generate a new node
	ASTNode *node = alloc(current_pos, groups);
	
	// Advance syntax pointer by the total amount matched
	syntax += matches[0].rm_eo;
	
	return node;
}

// Return true to force this node to exit on a newline
bool ASTNode::exit_on_newline() const {
	return false;
}

// Get a reference to the shared_ptr child object pointing to the given pointer
std::shared_ptr<ASTNode> &ASTNode::get_shared_ptr(ASTNode *to) {
	auto it = children.begin();
	for(; it != children.end(); it++)
		if(it->get() == to) break;
	return *it;
}

// Add a child and update its parent
void ASTNode::add_child(ASTNode *child) {
	child->parent = this;
	children.emplace_back(child);
}

void ASTNode::add_child(std::shared_ptr<ASTNode> child) {
	child->parent = this;
	children.push_back(child);
}

// Consume syntax, generate child nodes, and move up/down the syntax tree
ASTNode *ASTNode::consume(ASTNode *&current_node, NodePos &current_pos, const char *&syntax, bool new_line) {
	ASTNode *node;
	
	// All valid top-level AST nodes
	
	// Directives must be present at the start of a line
	if(new_line) {
		AST_CONSUME_ENTER(ASTDotInclude);
		AST_CONSUME_ENTER(ASTDotParam);
	}
	
	return nullptr;
}

}
}
