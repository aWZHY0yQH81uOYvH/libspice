#include "Circuit.hpp"

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
		// Numerator and denominator pointers may be NULL
		double num = t.num ? *t.num : 1;
		double den = t.den ? *t.den : 1;
		ret += t.coeff * num / den;
	}
	return ret;
}

void Circuit::gen_matrix() {
	reset();
	
	// Resize everything
	expr_mat.clear();
	expr_vec.clear();
	expr_mat.resize(nodes.size());
	expr_vec.resize(nodes.size());
	for(auto &v:expr_mat)
		v.resize(nodes.size());
	
	eval_mat.resize(nodes.size(), nodes.size());
	eval_vec = Eigen::VectorXd(nodes.size());
	node_voltage_vec = Eigen::VectorXd(nodes.size());
	
	// Each row and column correspond to a node
	for(size_t node_ind=0; node_ind<nodes.size(); node_ind++) {
		std::unique_ptr<Node> &n = nodes[node_ind];
		
		// Node is fixed to a voltage
		if(n->fixed) {
			// 1 * node voltage = fixed value
			expr_mat[node_ind][node_ind].push_back({1, NULL, NULL});
			expr_vec[node_ind].push_back({n->v, NULL, NULL});
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
				
				// Find which node each term of the current expression references, if any
				for(Term &t:ie) {
					ssize_t ref_node = -1;
					for(size_t x=0; x<nodes.size(); x++)
						if(t.num == &nodes[x]->v) {
							ref_node = x;
							break;
						}
					
					// Add term to the necessary position in the matrix
					if(ref_node >= 0) {
						// Remove the reference to the node from the term since
						// the matrix multiplication will include it automatically
						t.num = NULL;
						
						expr_mat[node_ind][ref_node].push_back(t);
					}
					
					// If it's a constant, put it in expr_vec
					else {
						// Coefficient must be inverted since the KCL term is moved
						// to the opposite side of the equation
						t.coeff *= -1;
						expr_vec[node_ind].push_back(t);
					}
				}
			}
		}
	}
	
	gen_matrix_pend = false;
	
	// Check if any of the components are dynamic and need to constantly update the circuit matrix
	update_matrix_pend = ONCE;
	for(auto &c:components)
		if(c->dynamic) {
			update_matrix_pend = ALWAYS;
			break;
		}
}

void Circuit::update_matrix() {
	// Evaluate the circuit definition matrix with current parameters
	// and convert to Eigen form
	
	for(size_t row=0; row<nodes.size(); row++)
		for(size_t col=0; col<nodes.size(); col++) {
			double value = eval_expr(expr_mat[row][col]);
			if(value)
				eval_mat.insert(row, col) = value;
		}
	
	for(size_t row=0; row<nodes.size(); row++)
		eval_vec[row] = eval_expr(expr_vec[row]);
	
	// Prepare the solver
	eval_mat.makeCompressed();
	mat_solver.analyzePattern(eval_mat);
	mat_solver.factorize(eval_mat);
	
	if(update_matrix_pend == ONCE)
		update_matrix_pend = NEVER;
}

void Circuit::sim_to_time(double stop) {
	if(gen_matrix_pend)
		gen_matrix();
	
	if(update_matrix_pend)
		update_matrix();
	
	// Solve the circuit matrix to get all node voltages
	node_voltage_vec = mat_solver.solve(eval_vec);
	
	// Update node object voltages
	for(size_t ind=0; ind<nodes.size(); ind++)
		nodes[ind]->v = node_voltage_vec[ind];
}
