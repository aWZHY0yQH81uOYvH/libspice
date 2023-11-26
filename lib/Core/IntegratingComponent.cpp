#include "Core/IntegratingComponent.hpp"

#include "Core/Circuit.hpp"

#include <stdexcept>

namespace spice {

IntegratingComponent::IntegratingComponent(Circuit *parent, double value):
	Component(parent, value) {}

IntegratingComponent::IntegratingComponent(Circuit *parent, double value, double initial_cond):
	Component(parent, value) {
	set_initial_cond(initial_cond);
}

IntegratingComponent::IntegratingComponent(Circuit *parent, double value, Node *top, Node *bottom):
	Component(parent, value, top, bottom) {}

IntegratingComponent::IntegratingComponent(Circuit *parent, double value, double initial_cond, Node *top, Node *bottom):
	Component(parent, value, top, bottom) {
	set_initial_cond(initial_cond);
}

void IntegratingComponent::set_initial_cond(double value) {
	parent_circuit->reset();
	initial_cond = value;
	initial_cond_specified = true;
}

void IntegratingComponent::gen_initial_cond() {
	throw std::runtime_error("Component requires explicitly specified initial condition");
}

Expression IntegratingComponent::dydt_expr() const {
	return {};
}

bool IntegratingComponent::is_dynamic() const {
	return true;
}

}
