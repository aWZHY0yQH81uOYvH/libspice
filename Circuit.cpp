#include "Circuit.hpp"
#include "Node.hpp"
#include "Component.hpp"

#include <stdexcept>
#include <map>

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
	if(period)
		save_period = period;
	
	for(auto &c:components)
		c->save = true;
	
	for(auto &n:nodes)
		n->save = true;
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

void Circuit::gen_matrix() {
	reset();
	
	// Make sure all components are connected properly
	for(std::unique_ptr<Component> &c:components)
		if(!c->fully_connected())
			throw std::runtime_error("Components not fully connected");
	
	// Figure out how many variables we have
	size_t n_nodes = nodes.size();
	n_vars = n_nodes;
	
	// Set the index in each node so going from node pointer to index is faster
	// Also create map of the pointers to each node's voltage to the node index
	// for faster association from within components' returned expressions
	node_map.clear();
	for(size_t ind=0; ind<n_nodes; ind++) {
		nodes[ind]->ind = ind;
		node_map.emplace(&nodes[ind]->v, ind);
	}
	
	// Count how many voltage sources there are and create an index of them
	// Each voltage source will get an additional variable that represents
	// the current through it
	vsource_map.clear();
	for(auto &c:components)
		if(c->v_expr().size())
			vsource_map.emplace(c.get(), n_vars++);
	
	// Resize everything
	// TODO: make expression matrix sparse (use std::map?)
	expr_mat.clear();
	expr_vec.clear();
	expr_mat.resize(n_vars);
	expr_vec.resize(n_vars);
	for(auto &v:expr_mat)
		v.resize(n_vars);
	
	eval_mat.resize(n_vars, n_vars);
	eval_vec = Eigen::VectorXd(n_vars);
	solved_vec = Eigen::VectorXd(n_vars);
	
	// TODO: separate alloc_matrix from gen_matrix to enable faster
	// term expression regeneration for highly nonlinear elements like
	// MOSFETs
	// Store each term in the expression matrix as a pair with component pointer (or just dynamic flag since they'd all need to be re-done?)
	// and term, and keep a list of "dynamic" components so the ones that need to be
	// updated can quickly be removed and updated instead of re-doing the entire thing
	
	// The first rows and columns correspond to nodes
	for(size_t node_ind=0; node_ind<n_nodes; node_ind++) {
		std::unique_ptr<Node> &n = nodes[node_ind];
		
		// Node is fixed to a voltage
		if(n->fixed) {
			// 1 * node voltage = fixed value
			expr_mat[node_ind][node_ind].push_back({1.0, {}, {}});
			expr_vec[node_ind].push_back({n->v, {}, {}});
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
							expr_mat[node_ind][search_result->second].push_back(t);
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
			expr_mat[vsource->node_top   ->ind][extra_var_ind].push_back({-1.0, {}, {}});
		
		if(!vsource->node_bottom->fixed)
			expr_mat[vsource->node_bottom->ind][extra_var_ind].push_back({ 1.0, {}, {}});
		
		// Create extra equation defining the forced voltage difference
		// Node voltage at the negative side should have negative sign
		expr_mat[extra_var_ind][vsource->node_top   ->ind].push_back({ 1.0, {}, {}});
		expr_mat[extra_var_ind][vsource->node_bottom->ind].push_back({-1.0, {}, {}});
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
	
	eval_mat.setZero();
	for(size_t row=0; row<n_vars; row++)
		for(size_t col=0; col<n_vars; col++) {
			double value = eval_expr(expr_mat[row][col]);
			if(value)
				eval_mat.insert(row, col) = value;
		}
	
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

void Circuit::sim_to_time(double stop) {
	if(gen_matrix_pend)
		gen_matrix();
	
	if(update_matrix_pend)
		update_matrix();
	
	// Solve the circuit matrix to get all node voltages
	solved_vec = mat_solver.solve(eval_vec);
	if(mat_solver.info() != Eigen::Success)
		throw std::runtime_error("SparseLU solve: " + mat_solver.lastErrorMessage());
	
	// Update node object voltages
	for(size_t ind=0; ind<nodes.size(); ind++)
		nodes[ind]->v = solved_vec[ind];
	
	// Update component object voltages and currents
	for(auto &c:components) {
		c->v = c->node_top->v - c->node_bottom->v;
		
		// Voltage-defined component has its own current var
		if(c->v_expr().size())
			c->i = solved_vec[vsource_map.find(c.get())->second];
		
		// Otherwise evaluate the current expression
		else
			c->i = eval_expr(c->i_expr());
	}
	
	// TODO: only compute updates when needed for saving or when exiting this function
}
