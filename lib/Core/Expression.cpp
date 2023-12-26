#include "Core/Expression.hpp"

namespace spice {

double Term::eval() const {
	double num_d = coeff;
	double den_d = 1;
	
	if(func)
		num_d *= (*func)();
	
	for(const double *n:num)
		num_d *= *n;
	
	for(const double *d:den)
		den_d *= *d;
	
	// Check for divide by zero
	if(den_d == 0)
		throw std::invalid_argument("Division by zero");
	
	return num_d / den_d;
}

double Expression::eval() const {
	double ret = 0;
	
	for(const Term &t:*this)
		ret += t.eval();
	
	return ret;
}

}
