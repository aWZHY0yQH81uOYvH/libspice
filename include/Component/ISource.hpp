/*
	It's a current source
*/

#ifndef ISOURCE_HPP
#define ISOURCE_HPP

#include "Core/Component.hpp"

class ISource: public Component {
	// Inherit constructor
	using Component::Component;
	
	virtual Expression i_expr() const;
};

#endif
