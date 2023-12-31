#include "Parser/Nodes/ASTExprOperator.hpp"
#include "Parser/Nodes/ASTExpression.hpp"

#include <cassert>

namespace spice {
namespace parser {

const char * const ASTExprOperator::regex_str    = "^(\\*\\*|\\/|\\*|-|\\+|<=|>=|<|>|\\^|\\||&)";
const int          ASTExprOperator::regex_flags  = 0;
const size_t       ASTExprOperator::regex_groups = 1;

const std::string ASTExprOperator::op_lut[OP_MAX] = {
	"&",
	"|",
	"^",
	">",
	"<",
	">=",
	"<=",
	"+",
	"-",
	"*",
	"/",
	"**"
};

ASTExprOperator::ASTExprOperator(NodePos pos, std::vector<std::string> &tokens): ASTExpression(pos) {
	const std::string &op = tokens.at(0);
	
	int op_ind = -1;
	for(int x = 0; x < OP_MAX; x++)
		if(op_lut[x] == op) {
			op_ind = x;
			break;
		}
	
	assert(op_ind >= 0);
	
	op_type = (OPType)op_ind;
}

void ASTExprOperator::all_to_cpp(FileInfo &fi) const {
	std::string func_name;
	
	switch(op_type) {
		case OP_AND:
			func_name = "builtin::_and";
			break;
			
		case OP_OR:
			func_name = "builtin::_or";
			break;
			
		case OP_XOR:
			func_name = "builtin::_xor";
			break;
			
		case OP_POWER:
			func_name = "builtin::pow";
			break;
			
		default:
			break;
	}
	
	bool got_expr = false;
	
	for(auto &child:children) {
		ASTExpression *expr = dynamic_cast<ASTExpression*>(child.get());
		if(expr) {
			if(func_name.length()) {
				if(got_expr) {
					child->all_to_cpp(fi);
					*fi.out << ')';
				} else {
					*fi.out << func_name << '(';
					child->all_to_cpp(fi);
					*fi.out << ", ";
				}
			} else {
				if(got_expr) {
					*fi.out << op_lut[op_type] << ' ';
					child->all_to_cpp(fi);
				} else {
					child->all_to_cpp(fi);
					*fi.out << ' ';
				}
			}
			
			got_expr = true;
		} else child->all_to_cpp(fi);
	}
}

ASTExprOperator::OPType ASTExprOperator::get_op_type() const {
	return op_type;
}

}
}
