#include "Component/Inductor.hpp"
#include "Core/Node.hpp"

Expression Inductor::i_expr() const {
	// V = L * dI/dt
	// V = IR
	// Current source in parallel with a resistor
	//   Isource is old inductor current
	//   R = L/dt
	return {{ 1, {this->var}, {}},
	        { 1, {node_top->v,    parent_circuit->dt}, {&this->value}},
	        {-1, {node_bottom->v, parent_circuit->dt}, {&this->value}}};
}

Expression Inductor::dc_v_expr() const {
	if(initial_cond_specified)
		return {};
	// Initial condition is a short
	return {{0, {}, {}}};
}

Expression Inductor::dc_i_expr() const {
	// Initial condition is constant current
	if(initial_cond_specified)
		return {{1, {&this->initial_cond}, {}}};
	return {};
}

Expression Inductor::dydt_expr() const {
	// dI/dt = V/L
	return {{ 1, {node_top->v   }, {&this->value}},
	        {-1, {node_bottom->v}, {&this->value}}};
}

void Inductor::gen_initial_cond() {
	initial_cond = i;
}
