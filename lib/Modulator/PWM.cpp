#include "Modulator/PWM.hpp"
#include "Core/Circuit.hpp"

PWM::PWM(Circuit *parent_circuit, double l_value, double h_value, double freq, double duty, double phase):
	Modulator(parent_circuit), duty(duty), freq(freq), period(1/freq), phase(phase), phase_offset(phase/freq/360), l_value(l_value), h_value(h_value) {}

void PWM::reset() {
	cached_nct = 0;
	next_change_time();
	_apply(next_state ^ (duty > 0 && duty < 1));
}

void PWM::apply() {
	double t = parent_circuit->time();
	if(t > cached_nct + EPSILON) next_change_time();
	if(t + EPSILON < cached_nct) return;
	
	_apply(next_state);
}

bool PWM::continuous() const {
	return false;
}

void PWM::_apply(bool state) {
	for(auto &c:controlled)
		*c.first = state ^ (c.second & Inverted) ? h_value : l_value;
	parent_circuit->needs_update();
}

double PWM::next_change_time() {
	double t = parent_circuit->time();
	if(t + EPSILON < cached_nct) return cached_nct;
	
	double ncycles = (t + phase_offset)/period;
	long int_ncycles = Circuit::epsilon_floor(ncycles);
	double basetime = int_ncycles*period;
	
	// Generate falling edge if current state should be high, but only if duty cycle isn't 1
	if(ncycles - int_ncycles + EPSILON < duty) {
		cached_nct = basetime + period*duty;
		next_state = duty >= 1;
	}
	
	// Generate rising edge if current state should be low, but only if duty cycle isn't 0
	else {
		cached_nct = basetime + period;
		next_state = duty > 0;
	}
	
	cached_nct -= phase_offset;
	
	return cached_nct;
}

double PWM::next_period() {
	return (Circuit::epsilon_floor((parent_circuit->time() + phase_offset)/period) + 1)*period - phase_offset;
}

double PWM::get_duty() {return duty;}
double PWM::get_freq() {return freq;}
double PWM::get_period() {return period;}
double PWM::get_phase() {return phase;}

void PWM::set_duty(double d) {
	duty = d;
	cached_nct = 0;
}

void PWM::set_freq(double f) {
	freq = f;
	period = 1/f;
	phase_offset = period*phase/360;
	cached_nct = 0;
}

void PWM::set_period(double p) {
	period = p;
	freq = 1/p;
	phase_offset = period*phase/360;
	cached_nct = 0;
}

void PWM::set_phase(double p) {
	phase = p;
	phase_offset = period*phase/360;
	cached_nct = 0;
}
