/*
	It's a voltage source
*/

#pragma once

#include "Core/Component.hpp"

namespace spice {

class VSource: public Component {
	// Inherit constructor
	using Component::Component;
	
	virtual Expression v_expr() const;
};

}
