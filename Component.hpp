/*
	Generic circuit component base class
*/

#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include <vector>

#include "Node.hpp"

class Circuit;

// A "term" can hold a multiplier and references to a
// fractional component. All are multiplied together.
typedef struct {
	double coeff;
	double *num;
	double *den;
} Term;

// Sum of multiple terms
typedef std::vector<Term> Expression;

class Component {
protected:
	Circuit *parent_circuit;
	Node *node_top = NULL;
	Node *node_bottom = NULL;
	double value;
	
	// Set to true to force re-evaluation of the component current parameters at every timestep
	bool dynamic = false;
	
	// Return expressions representing the voltage and current across/through this component
	virtual Expression v_expr();
	virtual Expression i_expr();
	
	// Voltage across and current through this component (to be updated by the solver)
	// DO NOT USE IN EXPRESSIONS
	double v = 0, i = 0;
	
	// Voltage and current histories
	std::vector<double> _v_hist;
	std::vector<double> _i_hist;
	void save_hist();
	
	Component(Circuit *parent, double value);
	// TODO: add constructor with each node specified
	
public:
	virtual ~Component() {}
	
	// Update value (i.e. resistance, capacitance)
	void set_value(double v);
	
	// Voltage and current histories
	const std::vector<double> &v_hist();
	const std::vector<double> &i_hist();
	bool save = false;
	
	// Current voltage and current values
	double voltage();
	double current();
	
	// Connecting to a node
	Node *to(Node *n);
	Component *to(Component *c);
	
	friend class Circuit;
	friend class Node;
};

#endif
