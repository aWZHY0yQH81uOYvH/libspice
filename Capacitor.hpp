/*
	It's a capacitor
*/

#ifndef CAPACITOR_HPP
#define CAPACITOR_HPP

#include "IntegratingComponent.hpp"

class Capacitor: public IntegratingComponent {
	// Inherit constructor
	using IntegratingComponent::IntegratingComponent;
	
	virtual Expression i_expr();
	virtual Expression dydt_expr();
};

#endif
