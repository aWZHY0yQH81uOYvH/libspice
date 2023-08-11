#include "IntegratingComponent.hpp"

#include "Circuit.hpp"

IntegratingComponent::IntegratingComponent(Circuit *parent, double value, double initial_cond):
	Component(parent, value), initial_cond(initial_cond) {}

IntegratingComponent::IntegratingComponent(Circuit *parent, double value, Node *top, Node *bottom):
	Component(parent, value, top, bottom), initial_cond(0) {}

IntegratingComponent::IntegratingComponent(Circuit *parent, double value, double initial_cond, Node *top, Node *bottom):
	Component(parent, value, top, bottom), initial_cond(initial_cond) {}

void IntegratingComponent::set_initial_cond(double value) {
	parent_circuit->reset();
	initial_cond = value;
}

Expression IntegratingComponent::dydt_expr() {
	return {};
}

bool IntegratingComponent::is_dynamic() const {
	return true;
}
