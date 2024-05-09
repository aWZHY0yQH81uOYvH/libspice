/*
	Class representing an arbitrary circuit
*/

#pragma once

#include "Core/Expression.hpp"

#include <vector>
#include <memory>
#include <utility>
#include <unordered_map>
#include <functional>

#include <Eigen/Core>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#include <Eigen/Sparse>
#include <Eigen/SparseLU>
#pragma clang diagnostic pop

#include <gsl/gsl_odeiv2.h>

// "Small" for floating-point calculations
#define EPSILON 1e-15

namespace spice {

class Node;
class Component;
class TwoTerminalComponent;
class IntegratingComponent;
class Modulator;

class Circuit {
private:
	// Nodes in circuit
	std::vector<std::unique_ptr<Node>> nodes;
	
	// Components in circuit
	std::vector<std::unique_ptr<Component>> components;
	
	// Modulators that can control component values
	std::vector<std::unique_ptr<Modulator>> modulators;
	
	// Circuit representation
	struct Coordinate {
		size_t row, col;
		
		bool operator==(const Coordinate &other) const {
			return row == other.row && col == other.col;
		}
	};
	
	struct CoordinateHash {
		size_t operator()(const Coordinate &c) const {
			std::hash<size_t> hasher;
			return hasher(c.row) ^ hasher(~c.col);
		}
	};
	
	std::unordered_map<Coordinate, Expression, CoordinateHash> expr_mat;
	std::vector<Expression> expr_vec;
	size_t n_vars;
	
	// Helper function to iterate over components of a certain dynamic type
	template<typename T> void for_component_type(std::function<void(T*)> func) {
		for(auto &c:components) {
			T *comp_type = dynamic_cast<T*>(c.get());
			if(comp_type)
				func(comp_type);
		}
	}
	
	// Generate, update, or solve circuit matrix
	void gen_matrix();
	void update_matrix();
	void solve_matrix();
	
	// Run apply() for all modulators
	void apply_modulators();
	
	// Return the index of the node in solved_vec given the node itself or its evaluation variable reference
	// Return -1 if it doesn't exist
	ssize_t node_index(const Node *node) const;
	ssize_t node_index(const double *var) const;
	
	// Evaluated circuit representation matrix
	Eigen::SparseMatrix<double> eval_mat;
	Eigen::VectorXd eval_vec;
	Eigen::VectorXd solved_vec;
	
	// Eigen solver
	Eigen::SparseLU<Eigen::SparseMatrix<double>> mat_solver;
	
	// If we need to re-generate the matrix
	bool gen_matrix_pend = true;
	
	// Time
	double t;
	
	// Time step (dynamic, will point into driver object)
	double *_dt = nullptr;
	
	// Timestep limits
	const double min_ts, max_ts;
	
	// Absolute error and relative error
	const double max_e_abs, max_e_rel;
	
	// ODE solver algorithm
	const gsl_odeiv2_step_type * const stepper_type;
	
	// GSL diff EQ solver driver object
	gsl_odeiv2_driver *driver = nullptr;
	
	// GSL diff EQ solver system
	gsl_odeiv2_system system;
	
	// Diff EQ state
	std::vector<double> deq_state;
	
	// dydt expressions
	std::vector<Expression> dydt_exprs;
	
	// Diff EQ system evaluation function
	static int system_function(double t, const double y[], double dydt[], void *params);
	
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
		topology_changed();
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
	double next_save_time() const;
	
	// Next modulator change time
	double next_modulator_time() const;
	
	// Next step that will be taken in sim_to_time
	double next_step_duration() const;
	double next_step_time() const;
	
	// Save states of all components and nodes (if enabled)
	void save_states();
	
	// Reset everything
	void reset();
	
	// Get current time
	double time() const;
	
	// Get pointer to internal time step
	const double *dt() const;
	
	// Simulate
	void sim_to_time(double stop, bool single_step = false);
	void sim_single_step();
	
	// DC solution for generating steady-state
	void compute_dc_solution();
	
	// Simulation mode which controls how components are represented
	enum {
		DC_ANALYSIS,
		TRANSIENT_ANALYSIS
	} simulation_mode = DC_ANALYSIS;
	
	// Indicate that the circuit topology has changed so the matrix needs to be re-generated
	void topology_changed();
	
	// Inexact floor function
	static long epsilon_floor(double x);
	
	// Inexact equals
	static bool epsilon_equals(double x, double y);
	
	friend class Node;
};

}
