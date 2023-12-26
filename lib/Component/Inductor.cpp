#include "Component/Inductor.hpp"
#include "Core/Circuit.hpp"
#include "Core/Node.hpp"

namespace spice {

Expression Inductor::tran_v_expr() const {
	return {};
}

Expression Inductor::tran_i_expr() const {
	// V = L * dI/dt
	// V = IR
	// Current source in parallel with a resistor
	//   Isource is old inductor current
	//   R = L/dt
	return {{ 1, {this->var}, {}},
	        { 1, {node_top->v, parent_circuit->dt}, {&this->value}},
	        {-1, {node_bot->v, parent_circuit->dt}, {&this->value}}};
}

Expression Inductor::dc_v_expr() const {
	if(initial_cond_specified)
		return {};
	// Initial condition is a short
	return {{0.0}};
}

Expression Inductor::dc_i_expr() const {
	// Initial condition is constant current
	if(initial_cond_specified)
		return {{&initial_cond}};
	return {};
}

Expression Inductor::dydt_expr() const {
	// dI/dt = V/L
	return {{ 1, {node_top->v}, {&this->value}},
	        {-1, {node_bot->v}, {&this->value}}};
}

void Inductor::gen_initial_cond() {
	initial_cond = current();
}

}
