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

#include <gsl/gsl_odeiv2.h>

// "Small" for floating-point calculations
#define EPSILON 1e-15

class Node;
class Component;
class IntegratingComponent;
class Modulator;

// A "term" can hold a multiplier and references to a
// fractional component. All are multiplied together.
typedef struct {
	double coeff;
	std::vector<const double *> num;
	std::vector<const double *> den;
} Term;

// Sum of multiple terms
typedef std::vector<Term> Expression;

class Circuit {
private:
	// Nodes in circuit
	std::vector<std::unique_ptr<Node>> nodes;
	
	// Components in circuit
	std::vector<std::unique_ptr<Component>> components;
	
	// Modulators that can control component values
	std::vector<std::unique_ptr<Modulator>> modulators;
	
	// Circuit representation
	// expr_mat is sparse so needs a helper function for easy access
	std::map<std::pair<size_t, size_t>, Expression> expr_mat;
	Expression &expr_mat_helper(size_t row, size_t col);
	std::vector<Expression> expr_vec;
	size_t n_vars;
	
	// Generate, update, or solve circuit matrix
	void gen_matrix();
	void update_matrix();
	void solve_matrix();
	
	// Update voltage and current inside a component from solved values
	void update_component(Component *c);
	
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
	
	// Time
	double t;
	
	// Timestep limits
	double min_ts, max_ts;
	
	// Absolute error and relative error
	double max_e_abs, max_e_rel;
	
	// ODE solver algorithm
	const gsl_odeiv2_step_type *stepper_type;
	
	// GSL diff EQ solver driver object
	gsl_odeiv2_driver *driver = NULL;
	
	// GSL diff EQ solver system
	gsl_odeiv2_system system;
	
	// Diff EQ state
	std::vector<double> deq_state;
	
	// Diff EQ system evaluation function
	static int system_function(double t, const double y[], double dydt[], void *params);
	
	// Map of IntegratingComponents to their indicies in the GSL system
	std::map<IntegratingComponent*, size_t> int_comp_map;
	
	// Times when a save was performed
	std::vector<double> _save_times;
	
public:
	// Constructor for setting ODE max timestep, solver algorithm, and error limits
	Circuit(double min_ts = 1e-15, double max_ts = 1e-6, double max_e_abs = 1e-12, double max_e_rel = 1e-3, const gsl_odeiv2_step_type *stepper_type = gsl_odeiv2_step_rkf45);
	
	~Circuit();
	
	// Prevent copies from being made (because of many internal pointer references)
	Circuit(const Circuit&) = delete;
	
	// Create new nodes
	Node *add_node();
	Node *add_node(double v);
	
	// Create new components
	// (define here so templates can be created on demand)
	template<typename T, typename... Args> T *add_comp(Args&&... args) {
		gen_matrix_pend = true;
		T *c = new T(this, std::forward<Args>(args)...);
		components.emplace_back(c);
		return c;
	}
	
	// Create new modulators
	// (define here so templates can be created on demand)
	template<typename T, typename... Args> T *add_mod(Args&&... args) {
		T *c = new T(this, std::forward<Args>(args)...);
		modulators.emplace_back(c);
		return c;
	}
	
	// How often voltages and currents will be saved
	// Zero for at every computed timestep
	double save_period = 0;
	
	// Enable saving for all nodes and components
	void save_all(double period = -1);
	
	// Times when a save was performed
	const std::vector<double> &save_times();
	
	// Next save time
	double next_save_time();
	
	// Save states of all components and nodes (if enabled)
	void save_states();
	
	// Reset everything
	void reset();
	
	// Get current time
	double time();
	
	// Simulate
	void sim_to_time(double stop);
	
	// Indicate that something has changed and the circuit matrix needs to be re-evaluated
	// Shouldn't need to be called by user code
	void needs_update();
	
	// Timestep (dynamic, will point into driver object)
	// TODO: figure out how to make private or read only?
	double *dt = NULL;
	
	// Inexact floor function
	static long epsilon_floor(double x);
	
	// Inexact equals
	static bool epsilon_equals(double x, double y);
	
	friend class Node;
	friend class Component;
};

#endif
