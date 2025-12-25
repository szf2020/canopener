#include "canopener.h"
#include <vector>
#include <cassert>

using namespace canopener;

void test_DataView() {
	printf("- Dataview...\n");

	std::vector<uint8_t> v;
	v.resize(4);

	DataView d(v);

	d.setUint32(0,0x12345678,true);

	assert(d.getUint32(0,true)==0x12345678);
	assert(d.getUint32(0,false)==0x78563412);
}
