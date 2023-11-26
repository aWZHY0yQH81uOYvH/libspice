#include "Component/ISource.hpp"

namespace spice {

Expression ISource::i_expr() const {
	return {{1, {&this->value}, {}}};
}

}
