/*
	It's a voltage source
*/

#ifndef VSOURCE_HPP
#define VSOURCE_HPP

#include "Component.hpp"

class VSource: public Component {
	// Inherit constructor
	using Component::Component;
	
	virtual Expression v_expr() const;
};

#endif
