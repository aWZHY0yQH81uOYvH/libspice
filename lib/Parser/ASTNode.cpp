#include "Parser/ASTNode.hpp"

#include "Parser/Nodes/ASTComment.hpp"
#include "Parser/Nodes/ASTNumericLiteral.hpp"

#include <cassert>
#include <unordered_map>
#include <regex.h>

namespace spice {
namespace parser {

const char * const ASTNode::regex_str    = ".*";
const int          ASTNode::regex_flags  = 0;
const size_t       ASTNode::regex_groups = 0;

ASTNode::ASTNode(ASTNode *parent, NodePos pos): parent(parent), pos(pos) {}

ASTNode::ASTNode(ASTNode *parent, NodePos pos, std::vector<std::string> &tokens): ASTNode(parent, pos) {
	(void)tokens;
}

// Convert this node to C++ representation
std::string ASTNode::to_cpp() const {
	return "";
}

std::string ASTNode::to_hpp() const {
	return "";
}

// Convert this node and all children to C++ syntax
std::string ASTNode::all_to_cpp() const {
	std::string ret = to_cpp();
	for(auto &child:children)
		ret += child->all_to_cpp();
	return ret;
}

std::string ASTNode::all_to_hpp() const {
	std::string ret = to_hpp();
	for(auto &child:children)
		ret += child->all_to_hpp();
	return ret;
}

// Verify syntax validity of just this node; throw exception if not valid
void ASTNode::verify() const {}

// Verify this node and all children
void ASTNode::all_verify() const {
	verify();
	for(auto &child:children)
		child->all_verify();
}

// Throw a verify error
void ASTNode::verify_fail(std::string error) const {
	throw SyntaxException{pos, error};
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
// Construct, append, and return a new child node of the given type if its regex matches the syntax input
// Also moves the syntax pointer forwards
// Intended to only be called through check_match
ASTNode *ASTNode::check_match_core(NodePos &current_pos, const char *&syntax,
                                   const char *regex_str, int regex_flags, size_t regex_groups,
                                   std::type_index ti, std::function<ASTNode*(ASTNode*, NodePos, std::vector<std::string>&)> alloc) {
	// Cache compiled regexes
	static std::unordered_map<std::type_index, std::unique_ptr<regex_t>> re_cache;
	
	auto &re = re_cache[ti];
	
	// Compile the regex if it's not in the cache
	if(!re) {
		re.reset(new regex_t);
		int ret = regcomp(re.get(), regex_str, regex_flags);
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
	ASTNode *node = alloc(this, current_pos, groups);
	
	// Append node
	children.emplace_back(node);
	
	// Advance current_pos and the syntax pointer by the total amount matched
	current_pos.character += matches[0].rm_eo;
	syntax += matches[0].rm_eo;
	
	return node;
}

// Consume syntax, generate child nodes, and move up/down the syntax tree
ASTNode *ASTNode::consume(ASTNode *&current_node, NodePos &current_pos, const char *&syntax, bool new_line) {
	assert(current_node == this);
	
	check_match<ASTComment>(current_pos, syntax);
}

}
}
