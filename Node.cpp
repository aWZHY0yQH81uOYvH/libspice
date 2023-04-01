#include "Node.hpp"
#include "Circuit.hpp"
#include "Component.hpp"

#include <stdexcept>

Node::Node(Circuit *c): parent_circuit(c), fixed(false) {}

Node::Node(Circuit *c, double v): parent_circuit(c), fixed(true), v(v) {}

const std::vector<double> &Node::v_hist() {
	return _v_hist;
}

void Node::save_hist() {
	_v_hist.push_back(v);
}

double Node::voltage() {
	return v;
}

Component *Node::to(Component *c) {
	if(parent_circuit != c->parent_circuit)
		throw std::invalid_argument("Component not in the same circuit");
	
	// Circuit topology changed...
	parent_circuit->gen_matrix_pend = true;
	
	// Tell component about this node
	c->node_top = this;
	
	// Current exiting this node
	connections.emplace_back(c, false);
	return c;
}
