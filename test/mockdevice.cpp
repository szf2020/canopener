#include "canopener.h"
#include <unistd.h>
#include <stdio.h>

using namespace canopener;

int skipfirst() {
	char s[256];
	fgets(s,256,stdin);

	SlcanStdioBus bus(0,1);
	Device device(bus);
	device.setNodeId(5);

	device.insert(0x4001,0x33).setType(Entry::INT32);

	//for (int i=0; i<5; i++) {
		//printf("Started mockdevice...\r\n");
		fflush(stdout);
	/*	sleep(1);
	}*/

	while (1) {
		device.loop();
	}

	return 0;
}

int normal() {
	SlcanStdioBus bus(0,1);
	Device device(bus);
	device.setNodeId(5);

	device.insert(0x4001,0x33).setType(Entry::INT32);

	//for (int i=0; i<5; i++) {
		//printf("Started mockdevice...\r\n");
		fflush(stdout);
	/*	sleep(1);
	}*/

	while (1) {
		device.loop();
	}

	return 0;
}

int main(int argc, char **argv) {
	if (argc!=2) {
		printf("need case\n");
		exit(1);
	}

	if (!strcmp(argv[1],"normal"))
		return normal();

	if (!strcmp(argv[1],"skipfirst"))
		return skipfirst();

	printf("unknown case: %s\n",argv[1]);
	exit(1);
}
