#include "Component/Capacitor.hpp"
#include "Core/Circuit.hpp"
#include "Core/Node.hpp"

namespace spice {

Expression Capacitor::tran_v_expr() const {
	return {};
}

Expression Capacitor::tran_i_expr() const {
	// I = C * dV/dt
	// I = V/R
	// Voltage source in series with resistor
	//   Vsource is old cap voltage
	//   R = dt/C
	// Convert to Norton
	//   I = Vcap*C/dt
	//   R = dt/C
	return {{-1, {this->var,   &this->value}, {parent_circuit->dt}},
	        { 1, {node_top->v, &this->value}, {parent_circuit->dt}},
	        {-1, {node_bot->v, &this->value}, {parent_circuit->dt}}};
}

Expression Capacitor::dc_v_expr() const {
	// Initial condition is a voltage source
	if(initial_cond_specified)
		return {&initial_cond};
	// Otherwise an open circuit
	return {};
}

Expression Capacitor::dc_i_expr() const {
	return {};
}

Expression Capacitor::dydt_expr() const {
	// dV/dt = I/C
	// same as i_expr but divided by C
	return {{-1, {this->var  }, {parent_circuit->dt}},
	        { 1, {node_top->v}, {parent_circuit->dt}},
	        {-1, {node_bot->v}, {parent_circuit->dt}}};
}

void Capacitor::gen_initial_cond() {
	initial_cond = voltage();
}

}
