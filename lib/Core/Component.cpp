#include "Core/Component.hpp"

namespace spice {

Component::Component(Circuit *parent_circuit): parent_circuit(parent_circuit) {}

bool Component::fully_connected() const {
	return true;
}

void Component::save_hist() {}

void Component::clear_hist() {}

}
