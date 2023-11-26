#include "Component/VSource.hpp"

namespace spice {

Expression VSource::v_expr() const {
	return {{1, {&this->value}, {}}};
}

}
