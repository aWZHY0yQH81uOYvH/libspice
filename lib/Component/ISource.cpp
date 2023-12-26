#include "Component/ISource.hpp"

namespace spice {

Expression ISource::dc_i_expr() const {
	return {{1, {&this->value}, {}}};
}

}
