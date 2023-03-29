#include <stdio.h>

#include "Circuit.hpp"

int main() {
	Circuit c;
	
	Node *vcc  = c.add_node(5);
	Node *gnd  = c.add_node(0);
	Node *vout = c.add_node();
	
	Resistor *R1 = c.add_comp<Resistor>(100);
	Resistor *R2 = c.add_comp<Resistor>(300);
	
	vcc->to(R1)->to(vout)->to(R2)->to(gnd);
	
	c.sim_to_time(0);
	
	printf("Voltage: %f\n", vout->voltage());
	
	return 0;
}
