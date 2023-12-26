/*
	It's a resistor
*/

#pragma once

#include "Core/TwoTerminalComponent.hpp"

namespace spice {

class Resistor: public TwoTerminalComponent {
	// Inherit constructor
	using TwoTerminalComponent::TwoTerminalComponent;
	
	virtual Expression dc_i_expr() const;
};

}
