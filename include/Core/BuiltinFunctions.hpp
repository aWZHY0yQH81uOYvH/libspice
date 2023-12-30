/*
	Builtin SPICE functions
	https://ltwiki.org/?title=B_sources_%28complete_reference%29
	Missing some of the more weird ones (table, noise...)
*/

#pragma once

#include <cmath>
#include <string>
#include <utility>
#include <unordered_set>

namespace spice {
struct builtin {
	// Just using as a namespace that prevents redefinition errors
	builtin() = delete;
	
	static const std::unordered_set<std::string> &available_functions() {
		static const std::unordered_set<std::string> funcs{"sin", "cos", "tan", "arcsin", "asin", "arccos", "acos", "arctan", "atan", "atan2", "hypot", "sinh", "cosh", "tanh", "exp", "ln", "log", "log10", "sgn", "abs", "sqrt", "square", "pow", "pwr", "pwrs", "round", "_int", "floor", "ceil", "min", "max", "limit", "uplim", "dnlim", "uramp", "stp", "u", "buf", "inv"};
		
		return funcs;
	}
	
	// sine
	static inline double sin(double x) {
		return std::sin(x);
	}
	
	// cosine
	static inline double cos(double x) {
		return std::cos(x);
	}
	
	// tangent
	static inline double tan(double x) {
		return std::tan(x);
	}
	
	// arc sine
	static inline double arcsin(double x) {
		return std::asin(x);
	}
	
	static inline double asin(double x) {
		return std::asin(x);
	}
	
	// arc cosine
	static inline double arccos(double x) {
		return std::acos(x);
	}
	
	static inline double acos(double x) {
		return std::acos(x);
	}
	
	// arc tangent
	static inline double arctan(double x) {
		return std::atan(x);
	}
	
	static inline double atan(double x) {
		return std::atan(x);
	}
	
	// arc tangent of y/x (four quadrant)
	static inline double atan2(double y, double x) {
		return std::atan2(y, x);
	}
	
	// hypotenuse: sqrt(x*x+y*y)
	static inline double hypot(double y, double x) {
		return std::sqrt(x*x+y*y);
	}
	
	// hyperbolic sine
	static inline double sinh(double x) {
		return std::sinh(x);
	}
	
	// hyperbolic cosine
	static inline double cosh(double x) {
		return std::cosh(x);
	}
	
	// hyperbolic tangent
	static inline double tanh(double x) {
		return std::tanh(x);
	}
	
	// exponential e**x
	static inline double exp(double x) {
		return std::exp(x);
	}
	
	// natural logarithm
	static inline double ln(double x) {
		return std::log(x);
	}
	
	static inline double log(double x) {
		return std::log(x);
	}
	
	// base 10 logarithm
	static inline double log10(double x) {
		return std::log10(x);
	}
	
	// sign (0 if x == 0)
	static inline double sgn(double x) {
		if(x > 0) return 1;
		if(x < 0) return -1;
		return 0;
	}
	
	// absolute value
	static inline double abs(double x) {
		return std::abs(x);
	}
	
	// square root
	static inline double sqrt(double x) {
		return std::sqrt(x);
	}
	
	// square (x**2)
	static inline double square(double x) {
		return x*x;
	}
	
	// x**y
	static inline double pow(double x, double y) {
		return std::pow(x, y);
	}
	
	// abs(x)**y
	static inline double pwr(double x, double y) {
		return std::pow(std::abs(x), y);
	}
	
	// sgn(x)*abs(x)**y
	static inline double pwrs(double x, double y) {
		return sgn(x) * pwr(x, y);
	}
	
	// round to nearest integer
	static inline double round(double x) {
		return std::round(x);
	}
	
	// truncate to integer part of x
	// note: can't be the standard name of int
	static inline double _int(double x) {
		return std::trunc(x);
	}
	
	// integer equal or less than x
	static inline double floor(double x) {
		return std::floor(x);
	}
	
	// integer equal or greater than x
	static inline double ceil(double x) {
		return std::ceil(x);
	}
	
	// the lesser of x or y
	static inline double min(double x, double y) {
		return std::min(x, y);
	}
	
	// the greater of x or y
	static inline double max(double x, double y) {
		return std::max(x, y);
	}
	
	// intermediate value of x, y, and z, equivalent to min(max(x,y),z)
	static inline double limit(double x, double y, double z) {
		return std::min(std::max(x, y), z);
	}
	
	// the lesser of x or y with soft limit zone of z
	static inline double uplim(double x, double y, double z) {
		return (y-x < z) ? (y - z*exp((y-x-z)/z)) : x;
	}
	
	// the greater of x or y with soft limit zone of z
	static inline double dnlim(double x, double y, double z) {
		return (x-y < z) ? (y + z*exp((x-y-z)/z)) : x;
	}
	
	// x if x > 0, else 0.
	static inline double uramp(double x) {
		return x > 0 ? x : 0;
	}
	
	// unit step, 1 if x > 0, else 0
	static inline double stp(double x) {
		return x > 0 ? 1 : 0;
	}
	
	static inline double u(double x) {
		return stp(x);
	}
	
	// 1 if x > .5, else 0
	static inline bool buf(double x) {
		return x > 0.5;
	}
	
	static inline bool buf(bool x) {
		return x;
	}
	
	// 0 if x > .5, else 1
	static inline template<typename Tx> bool inv(Tx x) {
		return !buf(x);
	}
	
	// Adapt boolean behavior to possibly use floating-point arguments (non-standard SPICE functions; used by SPICE translator)
	static inline template<typename Tx, typename Ty> bool _and(Tx x, Ty y) {
		return buf(x) && buf(y);
	}
	
	static inline template<typename Tx, typename Ty> bool _or(Tx x, Ty y) {
		return buf(x) || buf(y);
	}
	
	static inline template<typename Tx, typename Ty> bool _xor(Tx x, Ty y) {
		return buf(x) ^ buf(y);
	}
	
	static inline double 
	
};
}
