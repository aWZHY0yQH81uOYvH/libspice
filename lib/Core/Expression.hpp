/*
	Expressions and terms
*/

#pragma once

#include <functional>

namespace spice {

// A "term" can hold a multiplier and references to a fractional component.
// Optionally, a function can also be provided to be called on evaluation.
// All are multiplied together.
struct Term {
	double coeff;
	std::vector<const double *> num;
	std::vector<const double *> den;
	std::function<double(void)> *func;
	
	Term(double coeff, std::vector<const double *> num = {}, std::vector<const double *> den = {}, std::function<double(void)> *func = nullptr):
		coeff(coeff), num(num), den(den), func(func) {}
	
	Term(const double *ref):
		Term(1.0, {ref}) {}
	
	Term(std::function<double(void)> *func):
		Term(1.0, {}, {}, func) {}
	
	double eval() const;
};

// Sum of multiple terms
struct Expression: public std::vector<Term> {
	using std::vector<Term>::vector;
	
	double eval() const;
};

}
