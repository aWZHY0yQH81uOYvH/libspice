#include "ISource.hpp"

Expression ISource::i_expr() {
	return {{1, &this->value, NULL}};
}
