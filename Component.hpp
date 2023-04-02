/*
	Generic circuit component base class
*/

#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include <vector>

#include "Circuit.hpp"

class Node;

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
	
	Component(Circuit *parent, double value);
	Component(Circuit *parent, double value, Node *top, Node *bottom);
	
public:
	virtual ~Component() {}
	
	// Check if both ends of the component are connected
	bool fully_connected();
	
	// Update value (i.e. resistance, capacitance)
	double get_value();
	void set_value(double v);
	
	// Voltage and current histories
	const std::vector<double> &v_hist();
	const std::vector<double> &i_hist();
	void save_hist();
	bool auto_save = false;
	
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
