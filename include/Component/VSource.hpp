/*
	It's a voltage source
*/

#pragma once

#include "Core/TwoTerminalComponent.hpp"

namespace spice {

class VSource: public TwoTerminalComponent {
	// Inherit constructor
	using TwoTerminalComponent::TwoTerminalComponent;
	
	virtual Expression dc_v_expr() const;
};

}
