#include "Modulator/Sine.hpp"
#include "Core/Circuit.hpp"

#include <cmath>

Sine::Sine(Circuit *parent_circuit, double freq, double amp, double dc_offset, double phase):
	Modulator(parent_circuit), freq(freq), amp(amp), dc_offset(dc_offset), phase(phase) {}

void Sine::apply() {
	double value = amp*sin(2*M_PI*freq*parent_circuit->time() + phase*M_PI/180) + dc_offset;
	for(auto &c:controlled)
		*c.first = value;
	parent_circuit->needs_update();
}

bool Sine::continuous() const {
	return true;
}
