/*
	Class representing a node in a circuit
*/

#pragma once

#include <utility>
#include <vector>
#include <unordered_map>

namespace spice {

class Circuit;
class TwoTerminalComponent;

class Node {
private:
	// Circuit we're part of
	Circuit *parent_circuit;
	
	// For nodes to be kept at a constant voltage
	const bool fixed;
	const double fixed_voltage = 0;
	
	// List of components connected to this node, and corresponding current directions
	std::unordered_map<TwoTerminalComponent*, bool> connections;
	
	// Voltage history
	std::vector<double> _v_hist;
	
	// Current voltage
	const double *_v = nullptr;
	
	// Disconnected node
	Node(Circuit *c);
	
	// Fixed-voltage node
	Node(Circuit *c, double v);
	
	// No copy constructor
	Node(const Node&) = delete;
	
public:
	// Voltage history
	const std::vector<double> &v_hist();
	void save_hist();
	bool auto_save = false;
	
	// Current voltage
	const double *v() const;
	double voltage() const;
	
	// For connecting to components
	TwoTerminalComponent *to(TwoTerminalComponent *c);
	
	friend class Circuit;
	friend class TwoTerminalComponent;
};

}
