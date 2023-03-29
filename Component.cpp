#include <stdexcept>

#include "Component.hpp"
#include "Circuit.hpp"

Component::Component(Circuit *c, double v): parent_circuit(c), value(v) {}

void Component::set_value(double v) {
	if(v != value) {
		value = v;
		if(!parent_circuit->update_matrix_pend)
			parent_circuit->update_matrix_pend = Circuit::ONCE;
	}
}

const std::vector<double> &Component::v_hist() {
	return _v_hist;
}

const std::vector<double> &Component::i_hist() {
	return _i_hist;
}

void Component::save_hist() {
	_v_hist.push_back(v);
	_i_hist.push_back(i);
}

double Component::voltage() {
	return v;
}

double Component::current() {
	return i;
}

// Default to an undefined component
Expression Component::v_expr() {
	return {};
}

Expression Component::i_expr() {
	return {};
}

Node *Component::to(Node *n) {
	if(parent_circuit != n->parent_circuit)
		throw std::invalid_argument("Node not in the same circuit");
	
	if(node_top == n)
		throw std::invalid_argument("Both ends connected to same node");
	
	// Circuit topology changed...
	parent_circuit->gen_matrix_pend = true;
	
	// Remember our connections
	node_bottom = n;
	
	// Current entering the node
	n->connections.emplace_back(this, true);
	return n;
}

Component *Component::to(Component *c) {
	if(parent_circuit != c->parent_circuit)
		throw std::invalid_argument("Component not in the same circuit");
	
	return this->to(parent_circuit->add_node())->to(c);
}
