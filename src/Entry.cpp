#include "canopener.h"
#include <cstring>

using namespace canopener;

Entry::Entry(uint16_t index_, uint8_t subindex_)
        :view(data) {
	index=index_;
	subindex=subindex_;
	setType(INT32);
}

int Entry::getTypeSize() {
	switch (type) {
        case INT8:
        case UINT8:
        case BOOL:
        	return 1;

        case INT16:
        case UINT16:
        	return 2;

        case INT32:
        case UINT32:
        case FLOAT32:
        	return 4;

        default:
        	return 0;
	}
}

Entry& Entry::setType(Type type_) {
	type=type_;
	data.resize(getTypeSize());
	return *this;
}

void Entry::setRawNum(uint64_t raw) {
	// store in little-endian, regardless of type
	for (size_t i = 0; i < data.size(); ++i)
		data[i] = static_cast<uint8_t>((raw >> (8 * i)) & 0xFF);
}

uint32_t Entry::getRawNum() {
    uint32_t val = 0;
    for (size_t i = 0; i < data.size() && i < 4; ++i)
        val |= uint32_t(data[i]) << (8 * i);

    return val;
}