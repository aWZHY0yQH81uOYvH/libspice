#include "Parser/Nodes/ASTExpression.hpp"
#include "Parser/Nodes/ASTNumericLiteral.hpp"
#include "Parser/Nodes/ASTIdentifier.hpp"
#include "Parser/Nodes/ASTExprParentheses.hpp"
#include "Parser/Nodes/ASTExprCurlyBraces.hpp"
#include "Parser/Nodes/ASTExprOperator.hpp"
#include "Parser/Nodes/ASTFunction.hpp"

namespace spice {
namespace parser {

// Check match for any expression parts that can stand on their own
ASTNode *ASTEmptyExpression::check_match_unary(NodePos &current_pos, const char *&syntax) {
	ASTNode *node = nullptr;
	
	          node = check_match<ASTNumericLiteral >(current_pos, syntax);
	if(!node) node = check_match<ASTFunction       >(current_pos, syntax);
	if(!node) node = check_match<ASTIdentifier     >(current_pos, syntax);
	if(!node) node = check_match<ASTExprParentheses>(current_pos, syntax);
	if(!node) node = check_match<ASTExprCurlyBraces>(current_pos, syntax);
	
	return node;
}

void ASTEmptyExpression::verify() const {
	throw SyntaxException(pos, "Expected expression");
}

ASTNode *ASTEmptyExpression::consume(ASTNode *&current_node, NodePos &current_pos, const char *&syntax, bool new_line) {
	(void)new_line;
	
	// Try to replace ourselves with actual expression content
	ASTNode *node = check_match_unary(current_pos, syntax);
	
	if(node) {
		// Transfer all of our children to the new object
		for(auto &child:children)
			node->add_child(child);
		
		// Reset our parent's reference to us to point to the new child, which will lead to our destruction
		node->parent = parent;
		parent->get_shared_ptr(this).reset(node);
		
		// Enter the new node
		current_node = node;
	}
	
	return node;
}

ASTNode *ASTExpression::consume(ASTNode *&current_node, NodePos &current_pos, const char *&syntax, bool new_line) {
	(void)new_line;
	
	// If we are already an established expression, only match operators
	ASTExprOperator *op = check_match<ASTExprOperator>(current_pos, syntax);
	
	if(op) {
		// If we are inside another operator...
		ASTExprOperator *parent_op = dynamic_cast<ASTExprOperator*>(parent);
		if(parent_op) {
			// If the new operator is a higher precedence than our parent operator
			// e.g. 5+3*
			// We're in the 3, just consumed the *
			if(op->get_op_type() > parent_op->get_op_type()) {
				// Take ourselves away from the parent and give to the new operator
				auto &our_shared_ptr = parent->get_shared_ptr(this);
				op->parent = parent;
				op->add_child(our_shared_ptr);
				our_shared_ptr.reset(op);
			}
			
			// If we are lower or the same precedence, add our entire parent op as a child to the new op,
			// and swap out our grandparent's reference to our parent op for the new op
			// e.g. 5*3+
			// We're in the 3, just consumed the +
			else {
				// Add our entire parent op as a child to the new op
				auto *grandparent = parent->parent;
				auto &parents_shared_ptr = grandparent->get_shared_ptr(parent);
				op->add_child(parents_shared_ptr);
				
				// Swap out our grandparent's reference to our parent op for the new op
				op->parent = grandparent;
				parents_shared_ptr.reset(op);
			}
		}
		
		// Otherwise transfer ourself into the new operator, point parent to the operator
		// e.g. 3+
		// We're in the 3, just consumed the +
		else {
			auto &our_shared_ptr = parent->get_shared_ptr(this);
			op->parent = parent;
			op->add_child(our_shared_ptr);
			our_shared_ptr.reset(op);
		}
		
		// Create a new empty expression in the operator, and enter that
		auto *empty_expr = new ASTEmptyExpression(op->pos);
		op->add_child(empty_expr);
		current_node = empty_expr;
	}
	
	return op;
}

}
}
