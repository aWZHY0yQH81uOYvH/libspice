#include <stdio.h>

#include "Circuit.hpp"

int main() {
	Circuit c;
	
	Node *gnd  = c.add_node(0);
	
	Resistor *R1 = c.add_comp<Resistor>(100);
	Resistor *R2 = c.add_comp<Resistor>(300);
	
	VSource *volt = c.add_comp<VSource>(5);
	
	gnd->to(volt)->to(R1)->to(R2)->to(gnd);
	volt->flip();
	
	c.sim_to_time(0);
	
	printf("R1 V = %f, I = %f\n", R1->voltage(), R1->current());
	printf("R2 V = %f, I = %f\n", R2->voltage(), R2->current());
	printf("VSource V = %f, I = %f\n", volt->voltage(), volt->current());
	
	return 0;
}
