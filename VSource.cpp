#include "VSource.hpp"

Expression VSource::v_expr() {
	return {{1, &this->value, NULL}};
}
