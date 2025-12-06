#include "canopener.h"
#include <cstring>

using namespace canopener;

Entry::Entry(uint16_t index_, uint8_t subindex_) {
	index=index_;
	subindex=subindex_;
	type=INT32;
}

Entry& Entry::setType(Type type_) {
	type=type_;
	return *this;
}
