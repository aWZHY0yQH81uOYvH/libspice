/*
	It's a capacitor
*/

#pragma once

#include "Core/IntegratingComponent.hpp"

namespace spice {

class Capacitor: public IntegratingComponent {
	// Inherit constructor
	using IntegratingComponent::IntegratingComponent;
	
	virtual Expression tran_v_expr() const;
	virtual Expression tran_i_expr() const;
	virtual Expression dc_v_expr() const;
	virtual Expression dc_i_expr() const;
	virtual Expression dydt_expr() const;
	
	virtual void gen_initial_cond();
};

}
