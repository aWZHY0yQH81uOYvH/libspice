#include "Modulator.hpp"

#include <limits>

Modulator::Modulator(Circuit *parent_circuit): parent_circuit(parent_circuit) {}

void Modulator::reset() {}

void Modulator::apply() {}

bool Modulator::continuous() const {
	return true;
}

double Modulator::next_change_time() {
	return std::numeric_limits<double>::max();
}
