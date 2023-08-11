/*
	Generic class for an energy-storing component
	defined with differential equations
*/

#ifndef INTEGRATING_COMPONENT_HPP
#define INTEGRATING_COMPONENT_HPP

#include "Component.hpp"

class IntegratingComponent: public Component {
protected:
	// Variable for integration
	const double *var = NULL;
	double initial_cond;
	
	// Expression to be integrated to find var
	virtual Expression dydt_expr();
	
	virtual bool is_dynamic() const;

public:
	IntegratingComponent(Circuit *parent, double value, double initial_cond = 0);
	IntegratingComponent(Circuit *parent, double value, Node *top, Node *bottom);
	IntegratingComponent(Circuit *parent, double value, double initial_cond, Node *top, Node *bottom);
	
	void set_initial_cond(double value);
	
	friend class Circuit;
};

#endif
