#include "Core/Node.hpp"
#include "Core/Circuit.hpp"
#include "Core/TwoTerminalComponent.hpp"

#include <stdexcept>

namespace spice {

Node::Node(Circuit *c): parent_circuit(c), fixed(false) {}

Node::Node(Circuit *c, double v): parent_circuit(c), fixed(true), fixed_voltage(v), v(&fixed_voltage) {}

const std::vector<double> &Node::v_hist() {
	return _v_hist;
}

void Node::save_hist() {
	_v_hist.push_back(voltage());
}

double Node::voltage() {
	return v ? *v : 0;
}

TwoTerminalComponent *Node::to(TwoTerminalComponent *c) {
	if(parent_circuit != c->parent_circuit)
		throw std::invalid_argument("Component not in the same circuit");
	
	// Circuit topology changed...
	parent_circuit->topology_changed();
	
	// Tell component about this node
	if(c->node_top)
		throw std::invalid_argument("Component already connected");
	c->node_top = this;
	
	// Current exiting this node
	connections[c] = false;
	return c;
}

}
