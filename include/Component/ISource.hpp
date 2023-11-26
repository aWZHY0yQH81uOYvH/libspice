/*
	It's a current source
*/

#pragma once

#include "Core/Component.hpp"

namespace spice {

class ISource: public Component {
	// Inherit constructor
	using Component::Component;
	
	virtual Expression i_expr() const;
};

}
