/*
	It's a current source
*/

#pragma once

#include "Core/TwoTerminalComponent.hpp"

namespace spice {

class ISource: public TwoTerminalComponent {
	// Inherit constructor
	using TwoTerminalComponent::TwoTerminalComponent;
	
	virtual Expression dc_i_expr() const;
};

}
