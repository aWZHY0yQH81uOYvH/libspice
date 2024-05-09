/*
	Generic two-terminal component base class
*/

#pragma once

#include <vector>

#include "Core/Component.hpp"
#include "Core/Expression.hpp"

namespace spice {

class Node;
class Modulator;

class TwoTerminalComponent: public Component {
protected:
	// Nodes that we're connected to
	Node *node_top = nullptr;
	Node *node_bot = nullptr;
	
	// Optional modulator that can control our value
	Modulator *mod = nullptr;
	
	// Generic value for simple component types
	double value;
	
	// Return expressions representing the voltage and current across/through this component
	// Automatically switches between different expressions based on simulation mode of parent circuit
	Expression v_expr() const;
	Expression i_expr() const;
	
	// V/I expressions used for solving DC initial steady-state
	virtual Expression dc_v_expr() const;
	virtual Expression dc_i_expr() const;
	
	// V/I expressions used for transient analysis
	virtual Expression tran_v_expr() const;
	virtual Expression tran_i_expr() const;
	
	// Expressions that represent this component's voltage and current values in the circuit
	// Set by the Circuit class
	Expression circuit_v_expr;
	Expression circuit_i_expr;
	
	// Voltage and current histories
	std::vector<double> _v_hist;
	std::vector<double> _i_hist;
	
	// Constructors
	TwoTerminalComponent(Circuit *parent, double value = 0);
	TwoTerminalComponent(Circuit *parent, Node *top, Node *bottom);
	TwoTerminalComponent(Circuit *parent, double value, Node *top, Node *bottom);
	TwoTerminalComponent(Circuit *parent, Modulator *m, int flags = 0);
	TwoTerminalComponent(Circuit *parent, Modulator *m, Node *top, Node *bottom);
	TwoTerminalComponent(Circuit *parent, Modulator *m, int flags, Node *top, Node *bottom);
	
	// No copy constructor
	TwoTerminalComponent(const TwoTerminalComponent&) = delete;
	
public:
	virtual ~TwoTerminalComponent() {}
	
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
	virtual void save_hist();
	virtual void clear_hist();
	
	// Current voltage and current values
	double voltage() const;
	double current() const;
	double power() const;
	
	// Connecting to a node
	Node *to(Node *n);
	TwoTerminalComponent *to(TwoTerminalComponent *c);
	void flip();
	
	// Return connected nodes
	Node *top() const;
	Node *bot() const;
	
	friend class Circuit;
	friend class Node;
};

}
