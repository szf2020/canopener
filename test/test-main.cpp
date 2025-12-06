#include <stdio.h>
#include <cassert>
#include "canopener.h"
#include <format>
#include <iostream>

using namespace canopener;

void test_cof() {
	printf("- Cof (Can Open Frame)...\n");
	cof_t cof;

	cof_init(&cof);
	cof_set(&cof,COF_TYPE,COF_TYPE_SDO_ABORT);
	cof_set(&cof,COF_NODE_ID,0x05);
    cof_set(&cof,COF_SDO_INDEX,0x4001);
    cof_set(&cof,COF_SDO_SUB,0x10);
	cof_set(&cof,COF_SDO_ABORT_CODE,0x06020000);

	char s[32];
	cof_to_slcan(&cof,s);

	//printf("f: %s\n",s);
	assert(!strcmp(s,"t58588001401000000206"));

	cof_t cof2;
	cof_from_slcan(&cof2,s);

	assert(cof_get(&cof2,COF_TYPE)==COF_TYPE_SDO_ABORT);
	assert(cof_get(&cof2,COF_DLC)==8);
    assert(cof_get(&cof,COF_SDO_INDEX)==0x4001);
    assert(cof_get(&cof,COF_SDO_SUB)==0x10);
	assert(cof_get(&cof,COF_SDO_ABORT_CODE)==0x06020000);

	//assert(cof_get(&cof2,COF_U32_0)==0x12345678);
}

void test_MockBus() {
	printf("- Bus can send and receive...\n");
	MockBus bus;
	cof_t a,b;

	bus.rxBufPushSlcan("t123411223344");

	bus.read(&b);
	char s[16];
	cof_to_slcan(&b,s);
	assert(!strcmp(s,"t123411223344"));

	cof_set(&b,COF_COB_ID,0x55);

	bus.write(&b);
	a=bus.txBuf.front();
	cof_to_slcan(&a,s);
	assert(!strcmp(s,"t055411223344"));
}

void test_Device_basic() {
	printf("- Device can be created...\n");
	MockBus bus;
	Device device(bus);

	device.insert(0x4000,0x00).setType(Entry::INT32).set(123);
	assert(device.at(0x4000,0x00).get<uint32_t>()==123);
	assert(device.at(0x4000).get<uint32_t>()==123);

}

void test_Device_expedited_write() {
	printf("- Works with expedited SDO write...\n");
	MockBus bus;
	Device device(bus);
	device.setNodeId(5);

	device.insert(0x4001,0x33).setType(Entry::INT32);

	//Write 0x12345678 to index 0x4001, sub-index 0x33
	bus.rxBufPushSlcan("t60582301403378563412");
	device.loop();
	//printf("%08x\n",device.at(0x4001,0x33).get<uint32_t>());
	assert(device.at(0x4001,0x33).get<uint32_t>()==0x12345678);

	std::string s=bus.txBufPopSlcan();
	//printf("%s\n",s.c_str());
	//assert(s=="t585460014033");
	assert(!bus.txBuf.size());

	// not for us...
	bus.rxBufPushSlcan("t60682301403378563412");
	device.loop();
	assert(!bus.txBuf.size());

	// non existing...
	bus.rxBufPushSlcan("t60582301403478563412");
	device.loop();
	std::string t=bus.txBufPopSlcan();
	assert(t=="t58588001403400000206");
	//printf("%s\n",t.c_str());

}

int main() {
	printf("Running tests...\n");

	test_cof();
	test_MockBus();
	test_Device_basic();
	test_Device_expedited_write();

	return 0;
}