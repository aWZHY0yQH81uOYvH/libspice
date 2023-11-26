#include "Component/Resistor.hpp"
#include "Core/Node.hpp"

namespace spice {

Expression Resistor::i_expr() const {
	// I = (Vtop - Vbottom)/R
	return {{ 1, {node_top->v},    {&this->value}},
	        {-1, {node_bottom->v}, {&this->value}}};
}

}
