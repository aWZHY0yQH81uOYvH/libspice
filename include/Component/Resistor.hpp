/*
	It's a resistor
*/

#pragma once

#include "Core/Component.hpp"

namespace spice {

class Resistor: public Component {
	// Inherit constructor
	using Component::Component;
	
	virtual Expression i_expr() const;
};

}
