# libspice: A C++ circuit simulation library

### Example
```c++
#include <stdio.h>

#include "SPICE.hpp"

int main() {
	Circuit c;

	Node *gnd = c.add_node(0);

	VSource *volt = c.add_comp<VSource>(5);
	Resistor *R1  = c.add_comp<Resistor>(1e3);
	Capacitor *C1 = c.add_comp<Capacitor>(100e-9);

	gnd->to(volt)->to(R1)->to(C1)->to(gnd);
	volt->flip();

	C1->set_initial_cond(0);

	c.save_all();

	c.sim_to_time(1e-3);

	for(size_t x = 0; x < c.save_times().size(); x++)
		printf("%e,%e\n", c.save_times()[x], C1->v_hist()[x]);
}
```

