#include "Core/TwoTerminalComponent.hpp"
#include "Core/Circuit.hpp"
#include "Core/Node.hpp"
#include "Core/Modulator.hpp"

#include <stdexcept>

namespace spice {

TwoTerminalComponent::TwoTerminalComponent(Circuit *c, double v): Component(c), value(v) {}

TwoTerminalComponent::TwoTerminalComponent(Circuit *c, Node *top, Node *bottom): TwoTerminalComponent(c, 0.0) {
	top->to(this)->to(bottom);
}

TwoTerminalComponent::TwoTerminalComponent(Circuit *c, double v, Node *top, Node *bottom): TwoTerminalComponent(c, v) {
	top->to(this)->to(bottom);
}

TwoTerminalComponent::TwoTerminalComponent(Circuit *c, Modulator *m, int flags): TwoTerminalComponent(c, 0.0) {
	set_value(m, flags);
}

TwoTerminalComponent::TwoTerminalComponent(Circuit *c, Modulator *m, Node *top, Node *bottom): TwoTerminalComponent(c, m, 0) {
	top->to(this)->to(bottom);
}

TwoTerminalComponent::TwoTerminalComponent(Circuit *c, Modulator *m, int flags, Node *top, Node *bottom): TwoTerminalComponent(c, m, flags) {
	top->to(this)->to(bottom);
}

bool TwoTerminalComponent::fully_connected() const {
	return node_top && node_bot;
}

double TwoTerminalComponent::get_value() const {
	return value;
}

void TwoTerminalComponent::set_value(double v) {
	if(mod)
		throw std::logic_error("Component's value is already controlled by a modulator");
	
	value = v;
}

void TwoTerminalComponent::set_value(Modulator *m, int flags) {
	remove_mod();
	mod = m;
	mod->controlled[&value] = flags;
}

void TwoTerminalComponent::remove_mod() {
	if(mod) {
		mod->controlled.erase(&value);
		mod = nullptr;
	}
}

const std::vector<double> &TwoTerminalComponent::v_hist() {
	return _v_hist;
}

const std::vector<double> &TwoTerminalComponent::i_hist() {
	return _i_hist;
}

void TwoTerminalComponent::save_hist() {
	_v_hist.push_back(voltage());
	_i_hist.push_back(current());
}

void TwoTerminalComponent::clear_hist() {
	_v_hist.clear();
	_i_hist.clear();
}

double TwoTerminalComponent::voltage() const {
	return circuit_v_expr.eval();
}

double TwoTerminalComponent::current() const {
	return circuit_i_expr.eval();
}

double TwoTerminalComponent::power() const {
	return voltage() * current();
}

// Return normal or DC expressions
Expression TwoTerminalComponent::v_expr() const {
	switch(parent_circuit->simulation_mode) {
		case Circuit::DC_ANALYSIS:
			return dc_v_expr();
			
		case Circuit::TRANSIENT_ANALYSIS:
			return tran_v_expr();
			
		default:
			return {};
	}
}

Expression TwoTerminalComponent::i_expr() const {
	switch(parent_circuit->simulation_mode) {
		case Circuit::DC_ANALYSIS:
			return dc_i_expr();
			
		case Circuit::TRANSIENT_ANALYSIS:
			return tran_i_expr();
			
		default:
			return {};
	}
}

// Default to an undefined component
Expression TwoTerminalComponent::dc_v_expr() const {
	return {};
}

Expression TwoTerminalComponent::dc_i_expr() const {
	return {};
}

// Default to the same behavior as DC analysis
Expression TwoTerminalComponent::tran_v_expr() const {
	return dc_v_expr();
}

Expression TwoTerminalComponent::tran_i_expr() const {
	return dc_i_expr();
}

Node *TwoTerminalComponent::to(Node *n) {
	if(parent_circuit != n->parent_circuit)
		throw std::invalid_argument("Node not in the same circuit");
	
	if(node_top == n)
		throw std::invalid_argument("Both ends connected to same node");
	
	// Circuit topology changed...
	parent_circuit->topology_changed();
	
	// Remember our connections
	if(node_bot)
		throw std::invalid_argument("Component already connected");
	node_bot = n;
	
	// Current entering the node
	n->connections[this] = true;
	return n;
}

TwoTerminalComponent *TwoTerminalComponent::to(TwoTerminalComponent *c) {
	if(parent_circuit != c->parent_circuit)
		throw std::invalid_argument("Component not in the same circuit");
	
	return this->to(parent_circuit->add_node())->to(c);
}

void TwoTerminalComponent::flip() {
	// Circuit topology changed...
	parent_circuit->topology_changed();
	
	// Flip component direction
	Node *temp = node_bot;
	node_bot = node_top;
	node_top = temp;
	
	// Also update node references to us
	if(node_bot)
		node_bot->connections[this] ^= 1;

	if(node_top)
		node_top->connections[this] ^= 1;
}

Node *TwoTerminalComponent::top() const {
	return node_top;
}

Node *TwoTerminalComponent::bot() const {
	return node_bot;
}

}
