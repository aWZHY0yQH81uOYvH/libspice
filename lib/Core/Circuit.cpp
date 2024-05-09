#include "Core/Circuit.hpp"
#include "Core/Node.hpp"
#include "Core/Component.hpp"
#include "Core/TwoTerminalComponent.hpp"
#include "Core/IntegratingComponent.hpp"
#include "Core/Modulator.hpp"

#include <stdexcept>
#include <map>
#include <limits>

#include <gsl/gsl_errno.h>

namespace spice {

Circuit::Circuit(double min_ts, double max_ts, double max_e_abs, double max_e_rel, const gsl_odeiv2_step_type *stepper_type):
	min_ts(min_ts), max_ts(max_ts), max_e_abs(max_e_abs), max_e_rel(max_e_rel), stepper_type(stepper_type) {
	system.function = system_function;
	system.jacobian = nullptr;
	system.params = this;
}

Circuit::~Circuit() {
	if(driver)
		gsl_odeiv2_driver_free(driver);
}

Node *Circuit::add_node() {
	topology_changed();
	Node *n = new Node{this};
	nodes.emplace_back(n);
	return n;
}

Node *Circuit::add_node(double v) {
	// Return existing GND node if already created
	if(v == 0)
		return get_gnd_node();
	
	topology_changed();
	Node *n = new Node{this, v};
	nodes.emplace_back(n);
	return n;
}

Node *Circuit::get_gnd_node() {
	for(auto &node:nodes)
		if(node->fixed && node->fixed_voltage == 0)
			return node.get();
	
	topology_changed();
	Node *n = new Node{this, 0};
	nodes.emplace_back(n);
	return n;
}

// Enable saving for all nodes and components
void Circuit::save_all(double period) {
	if(period >= 0)
		save_period = period;
	
	for(auto &c:components)
		c->auto_save = true;
	
	for(auto &n:nodes)
		n->auto_save = true;
}

// Reset all states
void Circuit::reset() {
	gen_matrix_pend = true;
	simulation_mode = DC_ANALYSIS;
	t = 0;
	
	for(auto &c:components)
		c->clear_hist();
	
	for(auto &n:nodes)
		n->_v_hist.clear();
	
	for(auto &m:modulators)
		m->reset();
}

// Get current simulation time
double Circuit::time() const {
	return t;
}

// Get pointer to internal time step
const double *Circuit::dt() const {
	return _dt;
}

// Floor function that handles floating point imprecision
long Circuit::epsilon_floor(double x) {
	long normal_floor = x;
	if(normal_floor + 1 - x < EPSILON) return normal_floor + 1;
	return normal_floor;
}

// Check if floats are really close
bool Circuit::epsilon_equals(double x, double y) {
	return abs(x - y) < EPSILON;
}

// Return the next time we need to save
double Circuit::next_save_time() const {
	if(save_period) return (epsilon_floor(t/save_period) + 1)*save_period;
	return std::numeric_limits<double>::max();
}

// Return the next time a modulator changes state
double Circuit::next_modulator_time() const {
	double earliest = std::numeric_limits<double>::max();
	
	for(auto &m:modulators) {
		const double mod_change_time = m->next_change_time();
		if(mod_change_time < earliest)
			earliest = mod_change_time;
	}
	
	return earliest;
}

// Return the next time step length
double Circuit::next_step_duration() const {
	return std::max({min_ts,
	       std::min({max_ts,
	                 _dt ? *_dt : max_ts,
	                 next_save_time() - t,
	                 next_modulator_time() - t})});
}

// Return the next time we will step to
double Circuit::next_step_time() const {
	return t + next_step_duration();
}

const std::vector<double> &Circuit::save_times() {
	return _save_times;
}

void Circuit::save_states() {
	// Check if any components or nodes are flagged to have their state saved
	bool any_saved = false;
	
	for(auto &n:nodes)
		if(n->auto_save) {
			any_saved = true;
			n->save_hist();
		}
	
	for(auto &c:components)
		if(c->auto_save) {
			any_saved = true;
			c->save_hist();
		}
	
	if(any_saved)
		_save_times.push_back(t);
}

void Circuit::apply_modulators() {
	for(auto &m:modulators)
		m->apply();
}

// Return the index of the node in solved_vec given the node itself or its evaluation variable reference
// Return -1 if it doesn't exist
ssize_t Circuit::node_index(const Node *node) const {
	return node_index(node->v());
}

ssize_t Circuit::node_index(const double *var) const {
	// Find offset of node's voltage variable into solved_vec data area
	ssize_t diff = var - solved_vec.data();
	if(diff < 0 || (size_t)diff >= nodes.size()) return -1;
	return diff;
}

void Circuit::gen_matrix() {
	// Figure out how many variables we have
	size_t n_nodes = nodes.size();
	n_vars = n_nodes;
	
	system.dimension = 0;
	
	std::unordered_map<const TwoTerminalComponent*, size_t> vsource_map;
	
	for(auto &c:components) {
		const TwoTerminalComponent *ttc = dynamic_cast<const TwoTerminalComponent*>(c.get());
		const IntegratingComponent *ic  = dynamic_cast<const IntegratingComponent*>(c.get());
		
		// Make sure all components are connected properly
		if(!c->fully_connected())
			throw std::runtime_error("Components not fully connected");
		
		// Create a list of voltage sources
		// Each voltage source will get an additional variable that represents the current through it
		if(ttc && ttc->v_expr().size())
			vsource_map.emplace(ttc, n_vars++);
		
		// Count the number of IntegratingComponents
		if(ic)
			system.dimension++;
	}
	
	// Clear and resize circuit representation
	expr_mat.clear();
	expr_vec.clear();
	expr_vec.resize(n_vars);
	
	eval_mat.resize(n_vars, n_vars);
	eval_mat.setZero();
	eval_vec.resize(n_vars);
	solved_vec.resize(n_vars);
	
	dydt_exprs.resize(system.dimension);
	deq_state.resize(system.dimension);
	_dt = nullptr;
	
	// Set each node's voltage reference to the corresponding variable in solved_vec
	for(size_t ind = 0; ind < n_nodes; ind++)
		nodes[ind]->_v = &solved_vec[ind];
	
	if(simulation_mode == TRANSIENT_ANALYSIS && system.dimension) {
		// Allocate diff EQ driver
		if(driver)
			gsl_odeiv2_driver_free(driver);
		driver = gsl_odeiv2_driver_alloc_y_new(&system, stepper_type, max_ts, max_e_abs, max_e_rel);
		if(!driver)
			throw std::runtime_error("GSL driver allocation failed");
		
		gsl_odeiv2_driver_set_hmin(driver, min_ts);
		gsl_odeiv2_driver_set_hmax(driver, max_ts);
		_dt = &driver->h;
		*_dt = max_ts;
		
		// Initialize integrator state vector to initial conditions stored in components
		// and set each IntegratingComponent's integration variable reference
		size_t ic_ind = 0;
		for_component_type<IntegratingComponent>([&](IntegratingComponent *ic) {
			ic->var = deq_state.data() + ic_ind;
			deq_state[ic_ind] = ic->initial_cond;
			dydt_exprs[ic_ind] = ic->dydt_expr();
			ic_ind++;
		});
	}
	
	// Create voltage and current expressions for all TwoTerminalComponents
	for_component_type<TwoTerminalComponent>([&](TwoTerminalComponent *ttc) {
		// Voltage is always difference between top and bottom node voltages
		ttc->circuit_v_expr = {{ttc->node_top->v()}, {-1.0, {ttc->node_bot->v()}}};
		
		// Current is just the current expression, unless the component is voltage-defined and has its own current variable
		auto vi = vsource_map.find(ttc);
		if(vi == vsource_map.end())
			ttc->circuit_i_expr = ttc->i_expr();
		else
			ttc->circuit_i_expr = {&solved_vec[vi->second]};
	});
	
	// The first rows and columns correspond to nodes
	for(size_t node_ind = 0; node_ind < n_nodes; node_ind++) {
		std::unique_ptr<Node> &n = nodes[node_ind];
		
		// Node is fixed to a voltage
		if(n->fixed) {
			// 1 * node voltage = fixed value
			expr_mat[{node_ind, node_ind}].emplace_back(1.0);
			expr_vec[node_ind].emplace_back(n->fixed_voltage);
		}
		
		// Node is free to move; add currents to do KCL
		else {
			// Iterate through all components connected to this node
			for(auto &ci:n->connections) {
				Expression ie = ci.first->i_expr();
				
				// If current is leaving, invert coefficients of all terms
				if(!ci.second)
					for(Term &t:ie)
						t.coeff *= -1;
				
				// Find which node each term of the current expression numerator references, if any
				for(Term &t:ie) {
					ssize_t node_match = -1;
					
					// Iterate through all numerator parts
					for(auto num = t.num.begin(); num != t.num.end(); num++) {
						// Add term to the necessary position in the matrix if there is a reference
						node_match = node_index(*num);
						if(node_match >= 0) {
							// Remove the reference to the node from the term since
							// the matrix multiplication will include it automatically
							t.num.erase(num);
							expr_mat[{node_ind, (size_t)node_match}].push_back(t);
							break;
						}
					}
					
					// If it's a constant, put it in expr_vec
					if(node_match < 0) {
						// Coefficient must be inverted since the KCL term is moved
						// to the opposite side of the equation
						t.coeff *= -1;
						expr_vec[node_ind].push_back(t);
					}
				}
			}
		}
	}
	
	// Rest of the rows and cols correspond to the voltage-defined components
	for(auto vi:vsource_map) {
		const TwoTerminalComponent *vsource = vi.first;
		size_t extra_var_ind = vi.second;
		
		// Add 1 * current variables to connected nodes
		// but only if they aren't already a fixed voltage
		// (since equations for the fixed ones aren't KCL equations anymore)
		if(!vsource->node_top->fixed)
			expr_mat[{(size_t)node_index(vsource->node_top), extra_var_ind}].emplace_back(-1.0);
		
		if(!vsource->node_bot->fixed)
			expr_mat[{(size_t)node_index(vsource->node_bot), extra_var_ind}].emplace_back(1.0);
		
		// Create extra equation defining the forced voltage difference
		// Node voltage at the negative side should have negative sign
		expr_mat[{extra_var_ind, (size_t)node_index(vsource->node_top)}].emplace_back(1.0);
		expr_mat[{extra_var_ind, (size_t)node_index(vsource->node_bot)}].emplace_back(-1.0);
		expr_vec[extra_var_ind] = vsource->v_expr();
	}
	
	gen_matrix_pend = false;
}

void Circuit::update_matrix() {
	// Evaluate the circuit definition matrix with current parameters
	// and convert to Eigen form
	for(auto &expr:expr_mat)
		eval_mat.coeffRef(expr.first.row, expr.first.col) = expr.second.eval();
	
	for(size_t row = 0; row < n_vars; row++)
		eval_vec[row] = expr_vec[row].eval();
	
	// Prepare the solver
	if(!eval_mat.isCompressed()) {
		// Structure changed
		eval_mat.makeCompressed();
		mat_solver.analyzePattern(eval_mat);
	}
	
	mat_solver.factorize(eval_mat);
	if(mat_solver.info() != Eigen::Success)
		throw std::runtime_error("SparseLU factorize: " + mat_solver.lastErrorMessage());
}

void Circuit::topology_changed() {
	gen_matrix_pend = true;
	simulation_mode = DC_ANALYSIS;
}

void Circuit::solve_matrix() {
	// Recompute values in the matrix
	update_matrix();
	
	// Solve the circuit matrix to get all node voltages
	solved_vec = mat_solver.solve(eval_vec);
	if(mat_solver.info() != Eigen::Success)
		throw std::runtime_error("SparseLU solve: " + mat_solver.lastErrorMessage());
}

void Circuit::compute_dc_solution() {
	if(gen_matrix_pend)
		gen_matrix();
	
	apply_modulators();
	
	solve_matrix();
	
	// Update initial conditions for all IntegratingComponents that don't have it specified already
	for_component_type<IntegratingComponent>([](IntegratingComponent *ic) {
		if(!ic->initial_cond_specified)
			ic->gen_initial_cond();
	});
	
	save_states();
}

int Circuit::system_function(double t, const double y[], double dydt[], void *params) {
	Circuit *c = (Circuit*)params;
	
	// Save values for t and y
	double tempt = c->t;
	double tempy[c->deq_state.size()];
	memcpy(tempy, c->deq_state.data(), c->deq_state.size()*sizeof(double));
	
	// Ensure the values passed to this function are used
	memcpy(c->deq_state.data(), y, c->deq_state.size()*sizeof(double));
	c->t = t;
	
	// Update modulators
	for(auto &m:c->modulators)
		if(m->continuous())
			m->apply();
	
	// Solve matrix to keep all values up-to-date
	c->solve_matrix();
	
	// Evaluate all dydt expressions from each IntegratingComponent
	for(size_t ind = 0; ind < c->dydt_exprs.size(); ind++)
		dydt[ind] = c->dydt_exprs[ind].eval();
	
	// Restore old values t and y values for main circuit class
	c->t = tempt;
	memcpy(c->deq_state.data(), tempy, c->deq_state.size()*sizeof(double));
	
	return GSL_SUCCESS;
}

void Circuit::sim_to_time(double stop, bool single_step) {
	if(simulation_mode == DC_ANALYSIS) {
		compute_dc_solution();
		
		for(auto &m:modulators)
			m->reset();
		
		simulation_mode = TRANSIENT_ANALYSIS;
		gen_matrix_pend = true;
		
		if(single_step) return;
	}
	
	if(gen_matrix_pend) {
		gen_matrix();
		apply_modulators();
	}
	
	double step = _dt ? *_dt : max_ts;
	
	bool ran_step = false;
	
	while(t + EPSILON < stop && !(single_step && ran_step)) {
		ran_step = true;
		
		const double save_time = next_save_time();
		const double forced_end_time = std::min(save_time, next_modulator_time());
		step = next_step_duration();
		
		if(t + step > stop)
			step = stop - t;
		
		if(system.dimension) {
			*_dt = step;
			
			// Step diff EQs manually so we can get access to intermediate timesteps
			// (instead of using driver functions); essentially re-create gsl_odeiv2_evolve_apply
			gsl_odeiv2_evolve *e = driver->e;
			double *y = deq_state.data();
			
			// Save initial y values
			memcpy(e->y0, y, sizeof(double)*system.dimension);
			
			// Generate initial dydt if stepper needs it
			int step_status;
			if(driver->s->type->can_use_dydt_in) {
				if(e->count == 0)
					system_function(t, y, e->dydt_in, system.params);
				else
					memcpy(e->dydt_in, e->dydt_out, sizeof(double)*system.dimension);
				
				// Apply step
				step_status = gsl_odeiv2_step_apply(driver->s, t, *_dt, y, e->yerr, e->dydt_in, e->dydt_out, &system);
			}
			
			// Just apply step otherwise
			else
				step_status = gsl_odeiv2_step_apply(driver->s, t, *_dt, y, e->yerr, nullptr, e->dydt_out, &system);
			
			if(step_status == GSL_EFAULT)
				throw std::runtime_error("gsl_odeiv2_step_apply returned EFAULT");
			
			// If step succeeded, use automatic step size adjustment function
			// to possibly adjust the step for the next time
			if(step_status == GSL_SUCCESS) {
				e->count++;
				t += *_dt;
				gsl_odeiv2_control_hadjust(driver->c, driver->s, y, e->yerr, e->dydt_out, _dt);
			}
			
			// If step didn't succeed, try halving the timestep
			else {
				e->failed_steps++;
				
				// Can't make dt any smaller
				if(*_dt == min_ts)
					throw std::runtime_error("System does not converge at min timestep");
				
				*_dt /= 2;
				
				// Reset y
				memcpy(y, e->y0, sizeof(double)*system.dimension);
				
				continue;
			}
		}
		
		// If no diff EQs just solve matrix and jump to next interesting time
		else {
			t = forced_end_time;
			solve_matrix();
		}
		
		// Check if we need to save states
		// If save time is undefined, save whenever anything happens
		if(epsilon_equals(t, save_time) || save_time == std::numeric_limits<double>::max())
			save_states();
		
		// Run modulators
		apply_modulators();
	}
	
	// Update dt with new value for next time
	if(_dt) *_dt = step;
}

void Circuit::sim_single_step(double step_time) {
	if(step_time >= 0)
		sim_to_time(t + step_time, true);
	else
		sim_to_time(std::numeric_limits<double>::max(), true);
}

}
