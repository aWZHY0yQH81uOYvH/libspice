#include "Inductor.hpp"
#include "Node.hpp"

Expression Inductor::i_expr() {
	// V = L * dI/dt
	// V = IR
	// Current source in parallel with a resistor
	//   Isource is old inductor current
	//   R = L/dt
	return {{ 1, {this->var}, {}},
	        { 1, {node_top->v,    parent_circuit->dt}, {&this->value}},
	        {-1, {node_bottom->v, parent_circuit->dt}, {&this->value}}};
}

Expression Inductor::dydt_expr() {
	// dI/dt = V/L
	return {{ 1, {node_top->v   }, {&this->value}},
	        {-1, {node_bottom->v}, {&this->value}}};
}
