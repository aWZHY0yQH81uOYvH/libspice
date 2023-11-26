/*
	A generic object that will automatically control circuit components' values
*/

#ifndef MODULATOR_HPP
#define MODULATOR_HPP

#include <vector>
#include <utility>

class Circuit;

class Modulator {
protected:
	// Circuit that we're part of
	Circuit *parent_circuit;
	Modulator(Circuit *parent_circuit);
	
	// List of variables that we control and flags for each
	std::vector<std::pair<double*, int>> controlled;
	
	// Reset any internal state
	virtual void reset();
	
	// Apply changes to controlled variables
	virtual void apply();
	
	// Return true if this modulator can handle mon-monotonic time
	// Should be true for continuous functions, false for discontinuous ones
	// apply() will be called in the RK integration substeps if true
	virtual bool continuous() const;
	
public:
	virtual ~Modulator() {}
	
	// Next time at which an abrupt change will happen
	// return DBL_MAX if this always produces a constant or smoothly varying value
	virtual double next_change_time();
	
	friend class Circuit;
	friend class Component;
};

#endif
