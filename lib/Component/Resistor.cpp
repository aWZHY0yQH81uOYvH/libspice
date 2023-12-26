#include "Component/Resistor.hpp"
#include "Core/Node.hpp"

namespace spice {

Expression Resistor::dc_i_expr() const {
	// I = (Vtop - Vbottom)/R
	return {{ 1, {node_top->v}, {&this->value}},
	        {-1, {node_bot->v}, {&this->value}}};
}

}
