/*
	Generic circuit component base class
*/

#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include <vector>

#include "Core/Circuit.hpp"

class Node;
class Modulator;

class Component {
protected:
	// Circuit that we're part of
	Circuit *parent_circuit;
	
	// Nodes that we're connected to
	Node *node_top = NULL;
	Node *node_bottom = NULL;
	
	// Optional modulator that can control our value
	Modulator *mod = NULL;
	
	// Generic value for simple component types
	double value;
	
	// Return true to force re-evaluation of the component expressions at every timestep
	virtual bool is_dynamic() const;
	
	// Return expressions representing the voltage and current across/through this component
	Expression v_expr(bool dc) const;
	Expression i_expr(bool dc) const;
	virtual Expression v_expr() const;
	virtual Expression i_expr() const;
	
	// V/I expressions used for solving DC initial steady-state
	virtual Expression dc_v_expr() const;
	virtual Expression dc_i_expr() const;
	
	// Voltage across and current through this component (to be updated by the solver)
	// DO NOT USE IN EXPRESSIONS
	double v = 0, i = 0;
	
	// Voltage and current histories
	std::vector<double> _v_hist;
	std::vector<double> _i_hist;
	
	Component(Circuit *parent, double value);
	Component(Circuit *parent, double value, Node *top, Node *bottom);
	Component(Circuit *parent, Modulator *m, int flags = 0);
	Component(Circuit *parent, Modulator *m, Node *top, Node *bottom);
	Component(Circuit *parent, Modulator *m, int flags, Node *top, Node *bottom);
	
	// No copy constructor
	Component(const Component&) = delete;
	
public:
	virtual ~Component() {}
	
	// Check if both ends of the component are connected
	virtual bool fully_connected() const;
	
	// Update value (i.e. resistance, capacitance)
	double get_value() const;
	void set_value(double v);
	
	// Automatic modulator control over value
	void set_value(Modulator *m, int flags = 0);
	void remove_mod();
	
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