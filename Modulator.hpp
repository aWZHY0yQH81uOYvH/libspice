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
	
public:
	virtual ~Modulator() {}
	
	// Next time at which an abrupt change will happen
	// return DBL_MAX if this always produces a constant or smoothly varying value
	virtual double next_change_time();
	
	friend class Circuit;
	friend class Component;
};

#endif
