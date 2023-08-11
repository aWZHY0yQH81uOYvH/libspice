#include "ISource.hpp"

Expression ISource::i_expr() const {
	return {{1, {&this->value}, {}}};
}
