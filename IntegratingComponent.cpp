#include "IntegratingComponent.hpp"

#include "Circuit.hpp"

IntegratingComponent::IntegratingComponent(Circuit *parent, double value, double initial_cond):
	Component(parent, value), var(initial_cond) {}

IntegratingComponent::IntegratingComponent(Circuit *parent, double value, Node *top, Node *bottom):
	Component(parent, value, top, bottom), var(0) {}

IntegratingComponent::IntegratingComponent(Circuit *parent, double value, double initial_cond, Node *top, Node *bottom):
	Component(parent, value, top, bottom), var(initial_cond) {}

void IntegratingComponent::set_initial_cond(double value) {
	parent_circuit->reset();
	var = value;
}

Expression IntegratingComponent::dydt_expr() {
	return {};
}

bool IntegratingComponent::is_dynamic() {
	return true;
}
