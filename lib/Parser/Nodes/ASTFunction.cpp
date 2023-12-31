#include "Parser/Nodes/ASTFunction.hpp"
#include "Parser/Nodes/ASTIdentifier.hpp"
#include "Parser/Nodes/ASTExpression.hpp"
#include "Core/BuiltinFunctions.hpp"

#include <algorithm>

namespace spice {
namespace parser {

template<bool def> ASTFunction<def>::ASTFunction(NodePos pos, std::vector<std::string> &tokens): ASTExpression(pos) {
	name = tokens.at(0);
	
	// Some function names need special handling
	if(!def) {
		if(name == "int") name = "_int";
		else if(name == "if") if_statement = true;
	}
}

template<bool def> void ASTFunction<def>::verify() const {
	if(expecting_argument)
		throw SyntaxException((**args.rbegin())->pos, "Expected another argument");
	
	if(!closed)
		throw SyntaxException(pos, "Missing closing parenthesis for function");
	
	if(if_statement && args.size() != 3) {
		NodePos np = pos;
		if(args.size())
			np = (*args[std::min((size_t)3, args.size()-1)])->pos;
		throw SyntaxException(np, "Expected 3 arguments for if statement");
	}
}

template<bool def> void ASTFunction<def>::all_to_xpp(FileInfo &fi, bool include_namespace) const {
	FileInfo::AutoIndent indent(fi);
	
	if(def)
		*fi.out << "double ";
	
	// Add builtin::buf to convert first argument of if statement to boolean value
	if(if_statement)
		*fi.out << "(builtin::buf(";
	
	else {
		if(include_namespace) {
			// Namespace is builtin:: if function is known
			const auto &builtin_functions = builtin::available_functions();
			if(!def && builtin_functions.find(name) != builtin_functions.end())
				*fi.out << "builtin::";
			
			// Otherwise use the file name as the namespace
			else {
				auto ns = fi.path.filename();
				ns.replace_extension();
				*fi.out << ns.string() << "::";
			}
		}
		
		*fi.out << name << '(';
	}
	
	size_t arg_count = 0;
	
	for(auto &child:children) {
		ASTNode *arg;
		
		if(def)
			arg = dynamic_cast<ASTIdentifier*>(child.get());
		else
			arg = dynamic_cast<ASTExpression*>(child.get());
		
		if(arg) {
			if(if_statement) {
				// Add colon between arguments
				if(arg_count == 2)
					*fi.out << " : ";
				
				// Add parentheses around each argument
				if(arg_count == 1 || arg_count == 2)
					*fi.out << '(';
			}
			
			// Add argument types if this is a definition
			if(def)
				*fi.out << "double ";
			
			arg_count++;
		}
		
		child->all_to_cpp(fi);
		
		if(arg) {
			if(if_statement) {
				// Ternary if statement
				if(arg_count == 1)
					*fi.out << ") ? ";
				
				// Close argument parentheses
				if(arg_count == 2 || arg_count == 3)
					*fi.out << ')';
			}
			
			// Add comma separators between arguments
			else if(arg != (*args.rbegin())->get())
				*fi.out << ", ";
		}
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
	if(!closed && *syntax == ')') {
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
