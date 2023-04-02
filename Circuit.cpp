#include "Circuit.hpp"
#include "Node.hpp"
#include "Component.hpp"
#include "IntegratingComponent.hpp"

#include <stdexcept>
#include <map>
#include <limits>

#include <gsl/gsl_errno.h>

// "Small" for floating-point calculations
#define EPSILON 1e-15

Circuit::Circuit(double min_ts, double max_ts, double max_e_abs, double max_e_rel, const gsl_odeiv2_step_type *stepper_type):
	min_ts(min_ts), max_ts(max_ts), max_e_abs(max_e_abs), max_e_rel(max_e_rel), stepper_type(stepper_type) {
	system.function = system_function;
	system.jacobian = NULL;
	system.params = this;
}

Circuit::~Circuit() {
	if(driver)
		gsl_odeiv2_driver_free(driver);
}

Node *Circuit::add_node() {
	gen_matrix_pend = true;
	Node *n = new Node{this};
	nodes.emplace_back(n);
	return n;
}

Node *Circuit::add_node(double v) {
	gen_matrix_pend = true;
	Node *n = new Node{this, v};
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
	t = 0;
	
	for(auto &c:components) {
		c->_v_hist.clear();
		c->_i_hist.clear();
	}
	
	for(auto &n:nodes)
		n->_v_hist.clear();
}

// Get current simulation time
double Circuit::time() {
	return t;
}

double Circuit::eval_expr(const Expression &e) {
	double ret = 0;
	for(const Term &t:e) {
		double num = 1;
		double den = 1;
		
		for(const double *n:t.num)
			num *= *n;
		
		for(const double *d:t.den)
			den *= *d;
		
		// Check for divide by zero
		if(den == 0)
			throw std::invalid_argument("Division by zero");
		
		ret += t.coeff * num / den;
	}
	return ret;
}

// Return an expression object from the expression matrix
// Since it's a sparse matrix, it may need to be created
Expression &Circuit::expr_mat_helper(size_t row, size_t col) {
	auto coord = std::pair<size_t, size_t>(row, col);
	auto search_result = expr_mat.find(coord);
	if(search_result != expr_mat.end()) return search_result->second;
	return expr_mat.emplace(std::move(coord), Expression{}).first->second;
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
double Circuit::next_save_time() {
	if(save_period) return (epsilon_floor(t/save_period) + 1)*save_period;
	return std::numeric_limits<double>::max();
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
			update_component(c.get());
			c->save_hist();
		}
	
	if(any_saved)
		_save_times.push_back(t);
}

// Update voltage and current inside a component from solved values
void Circuit::update_component(Component *c) {
	c->v = *c->node_top->v - *c->node_bottom->v;
	
	// Voltage-defined component has its own current var
	if(c->v_expr().size())
		c->i = solved_vec[vsource_map.find(c)->second];
	
	// Otherwise evaluate the current expression
	else
		c->i = eval_expr(c->i_expr());
}

void Circuit::gen_matrix() {
	reset();
	
	// Make sure all components are connected properly
	for(std::unique_ptr<Component> &c:components)
		if(!c->fully_connected())
			throw std::runtime_error("Components not fully connected");
	
	// Figure out how many variables we have
	size_t n_nodes = nodes.size();
	n_vars = n_nodes;
	
	// Count how many voltage sources there are and create an index of them
	// Each voltage source will get an additional variable that represents
	// the current through it
	vsource_map.clear();
	for(auto &c:components)
		if(c->v_expr().size())
			vsource_map.emplace(c.get(), n_vars++);
	
	// Clear and resize circuit representation
	expr_mat.clear();
	expr_vec.clear();
	expr_vec.resize(n_vars);
	
	eval_mat.resize(n_vars, n_vars);
	eval_vec = Eigen::VectorXd(n_vars);
	solved_vec = Eigen::VectorXd(n_vars);
	
	// Set the index in each node so going from node pointer to index is faster
	// Also set each node's voltage reference to the corresponding variable in
	// solved_vec
	// Also create map of the pointers to each node's voltage to the node index
	// for faster association from within components' returned expressions
	node_map.clear();
	for(size_t ind=0; ind<n_nodes; ind++) {
		nodes[ind]->ind = ind;
		nodes[ind]->v = &solved_vec[ind];
		node_map.emplace(nodes[ind]->v, ind);
	}
	
	// TODO: separate alloc_matrix from gen_matrix to enable faster
	// term expression regeneration for highly nonlinear elements like
	// MOSFETs
	// Store each term in the expression matrix as a pair with component pointer (or just dynamic flag since they'd all need to be re-done?)
	// and term, and keep a list of "dynamic" components so the ones that need to be
	// updated can quickly be removed and updated instead of re-doing the entire thing
	
	// TODO: figure out when to reset driver
	
	// Count how many IntegratingComponents we have and give them indicies
	// Also allocate space for diff EQ state variables and initialize to initial states
	// already stored in component objects
	system.dimension = 0;
	int_comp_map.clear();
	deq_state.clear();
	for(auto &c:components) {
		IntegratingComponent *ic_ptr = dynamic_cast<IntegratingComponent*>(c.get());
		if(ic_ptr) {
			int_comp_map.emplace(ic_ptr, system.dimension++);
			deq_state.push_back(ic_ptr->initial_cond);
		}
	}
	
	// Set each IntegratingComponent's integration variable reference
	for(auto &ic:int_comp_map)
		ic.first->var = deq_state.data() + ic.second;
	
	// Allocate diff EQ driver if there are any IntegratingComponents
	if(system.dimension) {
		if(driver)
			gsl_odeiv2_driver_free(driver);
		driver = gsl_odeiv2_driver_alloc_y_new(&system, stepper_type, max_ts, max_e_abs, max_e_rel);
		if(!driver)
			throw std::runtime_error("GSL driver allocation failed");
		gsl_odeiv2_driver_set_hmin(driver, min_ts);
		gsl_odeiv2_driver_set_hmax(driver, max_ts);
		dt = &driver->h;
		*dt = max_ts;
	} else dt = NULL;
	
	// The first rows and columns correspond to nodes
	for(size_t node_ind=0; node_ind<n_nodes; node_ind++) {
		std::unique_ptr<Node> &n = nodes[node_ind];
		
		// Node is fixed to a voltage
		if(n->fixed) {
			// 1 * node voltage = fixed value
			expr_mat_helper(node_ind, node_ind).push_back({1.0, {}, {}});
			expr_vec[node_ind].push_back({*n->v, {}, {}});
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
					auto search_result = node_map.end();
					
					// Iterate through all numerator parts
					for(auto num = t.num.begin(); num != t.num.end(); num++)
						// Add term to the necessary position in the matrix if there is a reference
						if((search_result = node_map.find(*num)) != node_map.end()) {
							// Remove the reference to the node from the term since
							// the matrix multiplication will include it automatically
							t.num.erase(num);
							expr_mat_helper(node_ind, search_result->second).push_back(t);
							break;
						}
					
					// If it's a constant, put it in expr_vec
					if(search_result == node_map.end()) {
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
		Component *vsource = vi.first;
		size_t extra_var_ind = vi.second;
		
		// Add 1 * current variables to connected nodes
		// but only if they aren't already a fixed voltage
		// (since equations for the fixed ones aren't KCL equations anymore)
		if(!vsource->node_top   ->fixed)
			expr_mat_helper(vsource->node_top   ->ind, extra_var_ind).push_back({-1.0, {}, {}});
		
		if(!vsource->node_bottom->fixed)
			expr_mat_helper(vsource->node_bottom->ind, extra_var_ind).push_back({ 1.0, {}, {}});
		
		// Create extra equation defining the forced voltage difference
		// Node voltage at the negative side should have negative sign
		expr_mat_helper(extra_var_ind, vsource->node_top   ->ind).push_back({ 1.0, {}, {}});
		expr_mat_helper(extra_var_ind, vsource->node_bottom->ind).push_back({-1.0, {}, {}});
		expr_vec[extra_var_ind] = vsource->v_expr();
	}
	
	gen_matrix_pend = false;
	
	// Check if any of the components are dynamic and need to constantly update the circuit matrix
	update_matrix_pend = ONCE;
	for(auto &c:components)
		if(c->is_dynamic()) {
			update_matrix_pend = ALWAYS;
			break;
		}
}

void Circuit::update_matrix() {
	// Evaluate the circuit definition matrix with current parameters
	// and convert to Eigen form
	
	// TODO: only update the necessary parts of matrix
	
	eval_mat.setZero();
	for(auto &expr:expr_mat)
		eval_mat.insert(expr.first.first, expr.first.second) = eval_expr(expr.second);
	
	for(size_t row=0; row<n_vars; row++)
		eval_vec[row] = eval_expr(expr_vec[row]);
	
	// Prepare the solver
	eval_mat.makeCompressed();
	mat_solver.analyzePattern(eval_mat);
	mat_solver.factorize(eval_mat);
	if(mat_solver.info() != Eigen::Success)
		throw std::runtime_error("SparseLU factorize: " + mat_solver.lastErrorMessage());
	
	if(update_matrix_pend == ONCE)
		update_matrix_pend = NEVER;
}

void Circuit::solve_matrix() {
	// Recompute values in the matrix
	if(update_matrix_pend)
		update_matrix();
	
	// Solve the circuit matrix to get all node voltages
	solved_vec = mat_solver.solve(eval_vec);
	if(mat_solver.info() != Eigen::Success)
		throw std::runtime_error("SparseLU solve: " + mat_solver.lastErrorMessage());
}

int Circuit::system_function(double t, const double y[], double dydt[], void *params) {
	(void)t;
	(void)y; // Used in solve_matrix()
	Circuit *c = (Circuit*)params;
	
	// Solve matrix to keep all values up-to-date
	c->solve_matrix();
	
	// Evaluate all dydt expressions from each IntegratingComponent
	for(auto &ic:c->int_comp_map)
		dydt[ic.second] = eval_expr(ic.first->dydt_expr());
	
	return GSL_SUCCESS;
}

void Circuit::sim_to_time(double stop) {
	if(gen_matrix_pend)
		gen_matrix();
	
	double new_step = dt ? *dt : max_ts;
	
	// Make sure we have a save point at t = 0
	if(t == 0 && _save_times.size() == 0) {
		// Small dt for as close to a DC simulation as possible
		if(dt) *dt = min_ts;
		solve_matrix();
		save_states();
	}
	
	while(t + EPSILON < stop) {
		double save_time = next_save_time();
		
		// TODO: add calculation for modulators
		
		// Step to the soonest event
		double forced_end_time = std::min({save_time, stop});
		new_step = std::max(min_ts,
		           std::min({max_ts,
		                     new_step,
							 forced_end_time - t}));
		
		if(system.dimension) {
			*dt = new_step;
			
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
				step_status = gsl_odeiv2_step_apply(driver->s, t, *dt, y, e->yerr, e->dydt_in, e->dydt_out, &system);
			}
			
			// Just apply step otherwise
			else
				step_status = gsl_odeiv2_step_apply(driver->s, t, *dt, y, e->yerr, NULL, e->dydt_out, &system);
			
			if(step_status == GSL_EFAULT)
				throw std::runtime_error("gsl_odeiv2_step_apply returned EFAULT");
			
			// If step succeeded, use automatic step size adjustment function
			// to possibly adjust the step for the next time
			if(step_status == GSL_SUCCESS) {
				e->count++;
				t += *dt;
				gsl_odeiv2_control_hadjust(driver->c, driver->s, y, e->yerr, e->dydt_out, &new_step);
			}
			
			// If step didn't succeed, try halving the timestep
			else {
				e->failed_steps++;
				
				// Can't make dt any smaller
				if(*dt == min_ts)
					throw std::runtime_error("System does not converge at min timestep");
				
				new_step = *dt/2;
				
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
	}
	
	// Update component object voltages and currents for user to access
	for(auto &c:components)
		update_component(c.get());
	
	// Update dt with new value for next time
	if(dt) *dt = new_step;
}
