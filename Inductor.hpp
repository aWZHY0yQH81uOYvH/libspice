/*
	It's an inductor
*/

#ifndef INDUCTOR_HPP
#define INDUCTOR_HPP

#include "IntegratingComponent.hpp"

class Inductor: public IntegratingComponent {
	// Inherit constructor
	using IntegratingComponent::IntegratingComponent;
	
	virtual Expression i_expr() const;
	virtual Expression dc_v_expr() const;
	virtual Expression dc_i_expr() const;
	virtual Expression dydt_expr() const;
	
	virtual void gen_initial_cond();
};

#endif
