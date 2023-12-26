#include "Component/VSource.hpp"

namespace spice {

Expression VSource::dc_v_expr() const {
	return {{1, {&this->value}, {}}};
}

}
