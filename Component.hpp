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
	std::vector<const double *> num;
	std::vector<const double *> den;
} Term;

// Sum of multiple terms
typedef std::vector<Term> Expression;

class Component {
protected:
	// Circuit that we're part of
	Circuit *parent_circuit;
	
	// Nodes that we're connected to
	Node *node_top = NULL;
	Node *node_bottom = NULL;
	
	// Generic value for simple component types
	double value;
	
	// Return true to force re-evaluation of the component expressions at every timestep
	virtual bool is_dynamic();
	
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
	Component(Circuit *parent, double value, Node *top, Node *bottom);
	
public:
	virtual ~Component() {}
	
	// Check if both ends of the component are connected
	bool fully_connected();
	
	// Update value (i.e. resistance, capacitance)
	void set_value(double v);
	
	// Voltage and current histories
	const std::vector<double> &v_hist();
	const std::vector<double> &i_hist();
	bool save = false;
	
	// Current voltage and current values
	double voltage();
	double current();
	double power();
	
	// Connecting to a node
	Node *to(Node *n);
	Component *to(Component *c);
	void flip();
	
	friend class Circuit;
	friend class Node;
};

#endif
