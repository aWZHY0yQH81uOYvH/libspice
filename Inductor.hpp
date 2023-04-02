/*
	It's an inductor
*/

#ifndef INDUCTOR_HPP
#define INDUCTOR_HPP

#include "IntegratingComponent.hpp"

class Inductor: public IntegratingComponent {
	// Inherit constructor
	using IntegratingComponent::IntegratingComponent;
	
	virtual Expression i_expr();
	virtual Expression dydt_expr();
};

#endif
