/*
	It's a capacitor
*/

#ifndef CAPACITOR_HPP
#define CAPACITOR_HPP

#include "Core/IntegratingComponent.hpp"

class Capacitor: public IntegratingComponent {
	// Inherit constructor
	using IntegratingComponent::IntegratingComponent;
	
	virtual Expression i_expr() const;
	virtual Expression dc_v_expr() const;
	virtual Expression dc_i_expr() const;
	virtual Expression dydt_expr() const;
	
	virtual void gen_initial_cond();
};

#endif
