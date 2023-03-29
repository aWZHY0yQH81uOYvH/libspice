/*
	Class representing a node in a circuit
*/

#ifndef NODE_HPP
#define NODE_HPP

#include <utility>
#include <vector>

class Circuit;
class Component;

class Node {
private:
	// Circuit we're part of
	Circuit *parent_circuit;
	
	// For nodes to be kept at a constant voltage
	bool fixed;
	
	// List of components connected to this node, and corresponding current directions
	std::vector<std::pair<Component*, bool>> connections;
	
	// Voltage history
	std::vector<double> _v_hist;
	void save_hist();
	
	// Disconnected node
	Node(Circuit *c);
	
	// Fixed-voltage node
	Node(Circuit *c, double v);
	
public:
	// Voltage history
	const std::vector<double> &v_hist();
	bool save = false;
	
	// Current voltage
	double v; // TODO: find a way to make this private? needs to be public for component derived classes
	double voltage();
	
	// For connecting to components
	Component *to(Component *c);
	
	friend class Circuit;
	friend class Component;
};

#endif
