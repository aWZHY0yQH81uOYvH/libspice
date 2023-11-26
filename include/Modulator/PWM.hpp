/*
	PWM generator
*/

#ifndef PWM_HPP
#define PWM_HPP

#include "Core/Modulator.hpp"

class PWM: public Modulator {
private:
	PWM(Circuit *parent_circuit, double l_value, double h_value, double freq, double duty = 0, double phase = 0);
	
	// These affect the cached times, so they must be accessed through getters and setters
	double duty;
	double freq, period;
	double phase, phase_offset;
	
	virtual void reset();
	virtual void apply();
	virtual bool continuous() const;
	
	void _apply(bool state);
	
	// Cached next change time and what state to change to
	double cached_nct = 0;
	bool next_state = false;
	
public:
	double l_value;
	double h_value;
	
	virtual double next_change_time();
	
	// Time of the next full period
	double next_period();
	
	// Flags
	enum {
		Inverted = 0b1
	};
	
	double get_duty();
	double get_freq();
	double get_period();
	double get_phase();
	
	void set_duty(double d);
	void set_freq(double f);
	void set_period(double p);
	void set_phase(double p);
	
	
	friend class Circuit;
};

#endif
