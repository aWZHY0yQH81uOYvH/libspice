/*
	Modulate circuit parameters with a sine wave
*/

#ifndef SINE_HPP
#define SINE_HPP

#include "Core/Modulator.hpp"

class Sine: public Modulator {
private:
	Sine(Circuit *parent_circuit, double freq, double amp, double dc_offset = 0, double phase = 0);
	
	virtual void apply();
	virtual bool continuous() const;
	
public:
	double freq;
	double amp;
	double dc_offset;
	double phase;
	
	friend class Circuit;
};

#endif