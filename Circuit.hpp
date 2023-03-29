/*
	Class representing an arbitrary circuit
*/

#ifndef CIRCUIT_HPP
#define CIRCUIT_HPP

#include <vector>
#include <memory>
#include <utility>

#include <Eigen/Core>
#include <Eigen/Sparse>
#include <Eigen/SparseLU>

#include "Node.hpp"
#include "Component.hpp"

#include "Resistor.hpp"
#include "VSource.hpp"
#include "ISource.hpp"

class Circuit {
private:
	// Nodes in circuit
	std::vector<std::unique_ptr<Node>> nodes;
	
	// Components in circuit
	std::vector<std::unique_ptr<Component>> components;
	
	// Circuit representation
	std::vector<std::vector<Expression>> expr_mat;
	std::vector<Expression> expr_vec;
	size_t n_vars;
	
	// Map of variable indicies that correspond to voltage sources
	std::map<Component*, size_t> vsource_map;
	
	// Map of node voltage pointers to node indicies
	std::map<const double*, size_t> node_map;
	
	// Evaluate expressions used in circuit representation
	static double eval_expr(const Expression &e);
	
	// Evaluated circuit representation matrix
	Eigen::SparseMatrix<double> eval_mat;
	Eigen::VectorXd eval_vec;
	Eigen::VectorXd solved_vec;
	
	// Eigen solver
	Eigen::SparseLU<Eigen::SparseMatrix<double>> mat_solver;
	
	// If we need to re-generate or update the matrix
	bool gen_matrix_pend = true;
	enum {
		NEVER,
		ONCE,
		ALWAYS
	} update_matrix_pend = ONCE;
	
	// Timestep (dynamic)
	double ts;
	
	// Time
	double t;
	
public:
	// Create new nodes
	Node *add_node();
	Node *add_node(double v);
	
	// Create new components
	// (define here so templates can be created on demand)
	template<typename T, typename... Args> T *add_comp(Args&&... args) {
		T *c = new T(this, std::forward<Args>(args)...);
		components.emplace_back(c);
		return c;
	}
	
	// How often voltages and currents will be saved
	double save_period = 0;
	
	// Enable saving for all nodes and components
	void save_all(double period = 0);
	
	// Reset everything
	void reset();
	
	// Get current time
	double time();
	
	// Generate or update circuit matrix
	void gen_matrix();
	void update_matrix();
	
	// Simulate
	void sim_to_time(double stop);
	
	friend class Node;
	friend class Component;
};

#endif
