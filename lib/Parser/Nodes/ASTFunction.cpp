#include "Parser/Nodes/ASTFunction.hpp"
#include "Parser/Nodes/ASTIdentifier.hpp"
#include "Parser/Nodes/ASTExpression.hpp"

namespace spice {
namespace parser {

template<bool def> ASTFunction<def>::ASTFunction(NodePos pos, std::vector<std::string> &tokens): ASTExpression(pos) {
	name = tokens.at(0);
}

template<bool def> void ASTFunction<def>::verify() const {
	if(expecting_argument)
		throw SyntaxException((**args.rbegin())->pos, "Expected another argument");
	
	if(!closed)
		throw SyntaxException(pos, "Missing closing parenthesis for function");
}

template<bool def> void ASTFunction<def>::all_to_xpp(FileInfo &fi, bool include_namespace) const {
	if(def)
		*fi.out << "double ";
	
	if(include_namespace) {
		auto ns = fi.path.filename();
		ns.replace_extension();
		*fi.out << ns.string() << "::";
	}
	
	*fi.out << name << '(';
	
	bool first_arg = true;
	
	for(auto &child:children) {
		ASTNode *arg;
		
		if(def)
			arg = dynamic_cast<ASTIdentifier*>(child.get());
		else
			arg = dynamic_cast<ASTExpression*>(child.get());
		
		if(arg) {
			if(!first_arg)
				*fi.out << ", ";
			
			first_arg = false;
			
			if(def)
				*fi.out << "double ";
		}
		
		child->all_to_cpp(fi);
	}
	
	*fi.out << ')';
}

template<bool def> void ASTFunction<def>::all_to_cpp(FileInfo &fi) const {
	all_to_xpp(fi, true);
}

template<bool def> void ASTFunction<def>::all_to_hpp(FileInfo &fi) const {
	if(def)
		all_to_xpp(fi, false);
}

template<bool def> ASTNode *ASTFunction<def>::consume(ASTNode *&current_node, NodePos &current_pos, const char *&syntax, bool new_line) {
	(void)new_line;
	
	// Close when we get new parentheses
	if(*syntax == ')') {
		syntax++;
		closed = true;
		return this;
	}
	
	if(!closed) {
		// Look for new arguments if we have none or if there was a comma
		if(expecting_argument || args.size() == 0) {
			// Look only for identifiers if we are a function definition
			if(def) {
				ASTNode *id_match = check_match<ASTIdentifier>(current_pos, syntax);
				if(id_match) {
					args.push_back(&add_child(id_match));
					expecting_argument = false;
					return id_match;
				}
			}
			
			// Add an empty expression if this is a function call
			else {
				ASTEmptyExpression *expr = new ASTEmptyExpression(current_pos);
				current_node = expr;
				args.push_back(&add_child(expr));
				expecting_argument = false;
				return expr;
			}
		}
		
		// Look for a comma otherwise
		else if(*syntax == ',') {
			syntax++;
			expecting_argument = true;
			return this;
		}
	}
	
	// Turn back into a normal expression when we're closed (if we aren't a function definition)
	else if(!def) return ASTExpression::consume(current_node, current_pos, syntax, new_line);
	
	return nullptr;
}

template<bool def> std::string ASTFunction<def>::get_name() const {
	return name;
}

// Force instantiation of both types
template class ASTFunction<true>;
template class ASTFunction<false>;

}
}
