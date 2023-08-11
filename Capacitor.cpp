#include "Capacitor.hpp"
#include "Node.hpp"

Expression Capacitor::i_expr() const {
	// I = C * dV/dt
	// I = V/R
	// Voltage source in series with resistor
	//   Vsource is old cap voltage
	//   R = dt/C
	// Convert to Norton
	//   I = Vcap*C/dt
	//   R = dt/C
	return {{ 1, {this->var,      &this->value}, {parent_circuit->dt}},
	        { 1, {node_top->v,    &this->value}, {parent_circuit->dt}},
	        {-1, {node_bottom->v, &this->value}, {parent_circuit->dt}}};
}

Expression Capacitor::dydt_expr() const {
	// dV/dt = I/C
	// same as i_expr but divided by C
	return {{-1, {this->var     }, {parent_circuit->dt}},
	        {-1, {node_top->v   }, {parent_circuit->dt}},
	        { 1, {node_bottom->v}, {parent_circuit->dt}}};
}
