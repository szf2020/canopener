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

	cof_t *cof3=cof_create();
	cof_set(cof3,COF_TYPE,COF_TYPE_HEARTBEAT);

	assert(cof_get(cof3,COF_TYPE)==COF_TYPE_HEARTBEAT);

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

	device.insert(0x4000,0x03).setType(Entry::STRING).set(std::string("helloxxx"));
	std::string s=device.at(0x4000,0x03).get<std::string>();
	assert(s=="helloxxx");

	// Doesn't work for now...
	device.insert(0x4000,0x04).setType(Entry::STRING).set("hello");
	assert(device.at(0x4000,0x04).get<std::string>()=="hello");

	//assert(device.at(0x4000,0x03).get<std::string>()==std::string("hello"));

	device.insert(0x4000,0x05).setType(Entry::INT8).set(123);
	assert(device.at(0x4000,0x05).get<int8_t>()==123);
	assert(device.at(0x4000,0x05).get<uint32_t>()==123);
	assert(device.at(0x4000,0x05).get<float>()==123.0);

	device.at(0x4000,0x05).set("111");
	assert(device.at(0x4000,0x05).get<int>()==111);

	device.insert(0x4000,0x00).setType(Entry::UINT32).set(123);
	assert(device.at(0x4000,0x00).get<int>()==123);
	//assert(device.at(0x4000).get<uint32_t>()==123);
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

void test_Device_expedited_write16() {
	printf("- Works with expedited SDO write (16bit)...\n");
	MockBus bus;
	Device device(bus);
	device.setNodeId(5);

	device.insert(0x4001,0x33).setType(Entry::INT16);

	//Write 0x12345678 to index 0x4001, sub-index 0x33
	bus.rxBufPushSlcan("t60582301403378563412");
	device.loop();
	//printf("%08x\n",device.at(0x4001,0x33).get<uint32_t>());
	assert(device.at(0x4001,0x33).get<uint32_t>()==0x5678);
}

void test_castx() {
	printf("- castx...\n");

	int i=castx<uint32_t>(std::string("123"));
	assert(i==123);

	int j=castx<uint32_t, const char *>("123");
	assert(j==123);

	int k=castx<uint32_t>("123");
	assert(k==123);

	int l=castx<int>((int)1234);
	assert(l==1234);

	std::string s=castx<std::string>(123);
	assert(s=="123");

	std::string t=castx<std::string,std::string>(std::string("bla"));
	assert(t=="bla");
}

void test_BridgeBus();
void test_DataView();

int main() {
	printf("Running tests...\n");

	test_cof();
	test_MockBus();
	test_Device_basic();
	test_Device_expedited_write();
	test_Device_expedited_write16();
	test_BridgeBus();
	test_DataView();
	test_castx();

	return 0;
}