/*
	Generic circuit component base class
*/

#pragma once

namespace spice {

class Circuit;

class Component {
protected:
	// Circuit that we're part of
	Circuit *parent_circuit;
	
	Component(Circuit *parent);
	
	// No copy constructor
	Component(const Component&) = delete;
	
public:
	virtual ~Component() {}
	
	// Check if this component is correctly connected
	virtual bool fully_connected() const;
	
	// History saving (to be implemented separately for each type of component)
	virtual void save_hist();
	virtual void clear_hist();
	bool auto_save = false;
	
	friend class Circuit;
};

}
