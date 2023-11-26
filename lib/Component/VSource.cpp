#include "Component/VSource.hpp"

Expression VSource::v_expr() const {
	return {{1, {&this->value}, {}}};
}
