#include "Component.hpp"
#include "Circuit.hpp"
#include "Node.hpp"
#include "Modulator.hpp"

#include <stdexcept>

Component::Component(Circuit *c, double v): parent_circuit(c), value(v) {}

Component::Component(Circuit *c, double v, Node *top, Node *bottom): Component(c, v) {
	top->to(this)->to(bottom);
}

Component::Component(Circuit *c, Modulator *m, int flags): Component(c, 0.0) {
	set_value(m, flags);
}

Component::Component(Circuit *c, Modulator *m, Node *top, Node *bottom): Component(c, m, 0) {
	top->to(this)->to(bottom);
}

Component::Component(Circuit *c, Modulator *m, int flags, Node *top, Node *bottom): Component(c, m, flags) {
	top->to(this)->to(bottom);
}

bool Component::fully_connected() {
	return node_top && node_bottom;
}

double Component::get_value() {
	return value;
}

void Component::set_value(double v) {
	if(mod)
		throw std::logic_error("Component's value is already controlled by a modulator");
	
	if(v != value) {
		value = v;
		parent_circuit->needs_update();
	}
}

void Component::set_value(Modulator *m, int flags) {
	remove_mod();
	mod = m;
	mod->controlled.emplace_back(&value, flags);
}

void Component::remove_mod() {
	if(mod) {
		for(auto it = mod->controlled.begin(); it != mod->controlled.end(); it++)
			if(it->first == &value) {
				mod->controlled.erase(it);
				break;
			}
		mod = NULL;
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

double Component::power() {
	return v*i;
}

// Default to a non-dynamic component
bool Component::is_dynamic() {
	return false;
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
	if(node_bottom)
		throw std::invalid_argument("Component already connected");
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

void Component::flip() {
	// Circuit topology changed...
	parent_circuit->gen_matrix_pend = true;
	
	// Flip component direction
	Node *temp = node_bottom;
	node_bottom = node_top;
	node_top = temp;
	
	// Also update node references to us
	if(node_bottom)
		for(auto &conn:node_bottom->connections)
			if(conn.first == this) {
				conn.second ^= 1;
				break;
			}

	if(node_top)
		for(auto &conn:node_top->connections)
			if(conn.first == this) {
				conn.second ^= 1;
				break;
			}
}
