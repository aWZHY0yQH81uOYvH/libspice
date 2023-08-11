/*
	It's a resistor
*/

#ifndef RESISTOR_HPP
#define RESISTOR_HPP

#include "Component.hpp"

class Resistor: public Component {
	// Inherit constructor
	using Component::Component;
	
	virtual Expression i_expr() const;
};

#endif
