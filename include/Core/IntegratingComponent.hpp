/*
	Generic class for an energy-storing component
	defined with differential equations
*/

#pragma once

#include "Core/Component.hpp"

namespace spice {

class IntegratingComponent: public Component {
protected:
	// Variable for integration
	const double *var = NULL;
	double initial_cond = 0;
	
	// DC solution will be used to set initial_cond if false
	bool initial_cond_specified = false;
	
	// Set initial_cond from current component V/I values
	virtual void gen_initial_cond();
	
	// Expression to be integrated to find var
	virtual Expression dydt_expr() const;
	
	virtual bool is_dynamic() const;

public:
	IntegratingComponent(Circuit *parent, double value);
	IntegratingComponent(Circuit *parent, double value, double initial_cond);
	IntegratingComponent(Circuit *parent, double value, Node *top, Node *bottom);
	IntegratingComponent(Circuit *parent, double value, double initial_cond, Node *top, Node *bottom);
	
	virtual void set_initial_cond(double value);
	
	friend class Circuit;
};

}
