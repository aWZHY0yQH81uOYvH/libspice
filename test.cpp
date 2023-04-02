#include <stdio.h>

#include "SPICE.hpp"

int main() {
	Circuit c;
	
	Node *gnd = c.add_node(0);
	
	VSource *volt = c.add_comp<VSource>(0);
	
	Resistor *R1 = c.add_comp<Resistor>(10);
	Capacitor *C1 = c.add_comp<Capacitor>(1e-6);
	Inductor *L1 = c.add_comp<Inductor>(1e-3);
	
	gnd->to(volt)->to(R1)->to(C1)->to(L1)->to(gnd);
	volt->flip();
	
	L1->auto_save = true;
	volt->auto_save = true;
	
	c.sim_to_time(0.1e-3);
	volt->set_value(5);
	c.sim_to_time(1e-3);
	
	for(size_t x=0; x<c.save_times().size(); x++)
		printf("%e,%e,%e\n", c.save_times()[x], volt->v_hist()[x], L1->v_hist()[x]);
	
	return 0;
}
