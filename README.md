# libspice: A C++ circuit simulation library

### Minimal Example
```c++
#include <stdio.h>

#include "SPICE.hpp"

int main() {
	// Create a circuit with the default integrator, time step limits, and
	// precision settings
	Circuit c;

	// Create a named node, constrained to 0V (GND)
	// Nodes can also be free to change voltage if constructor is empty
	Node *gnd = c.add_node(0);

	// Create some two-terminal components with set values
	VSource *volt = c.add_comp<VSource>(5);
	Resistor *R1  = c.add_comp<Resistor>(1e3);
	Capacitor *C1 = c.add_comp<Capacitor>(100e-9);

	// Assemble circuit
	// Nodes are automatically created as needed, but can be explicitly created
	// (like with GND) to create multiple connections to one node
	gnd->to(volt)->to(R1)->to(C1)->to(gnd);
	
	// Flip voltage source so voltages are positive
	volt->flip();

	// Do not use DC operating point to determine initial condition for
	// capcitor charge; specify it as 0V
	C1->set_initial_cond(0);

	// Save currents and voltages for all components and nodes at every timestep
	// Alternatively, individual components can be told to save
	c.save_all();

	// Step simulation to 1ms
	c.sim_to_time(1e-3);

	// Print out timesteps and voltage across the capacitor
	for(size_t x = 0; x < c.save_times().size(); x++)
		printf("%e,%e\n", c.save_times()[x], C1->v_hist()[x]);
	
	// It is now possible to run some control algorithm, update values based on
	// the circuit state, and continue stepping...
}
```
