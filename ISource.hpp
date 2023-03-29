/*
	It's a current source
*/

#ifndef ISOURCE_HPP
#define ISOURCE_HPP

#include "Component.hpp"

class ISource: public Component {
	// Inherit constructor
	using Component::Component;
	
	Expression i_expr();
};

#endif
