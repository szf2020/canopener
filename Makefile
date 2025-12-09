WATCHSOURCES=test/*.cpp src/*.c* include/*.h include/canopener/*.h emcc_flags.rsp

all: bin/mockdevice bin/test dist/cof-defines.js dist/cof-wasm.wasm dist/cof-wasm.js

bin/mockdevice: $(WATCHSOURCES)
	g++ -obin/mockdevice -Iinclude src/*.c* test/mockdevice.cpp

bin/test: $(WATCHSOURCES)
	g++ -obin/test -Iinclude src/*.c* test/test-main.cpp

dist/cof-defines.js: $(WATCHSOURCES)
	node scripts/generate-cof-defines.js

dist/cof-wasm.wasm dist/cof-wasm.js: $(WATCHSOURCES)
	emcc @emcc_flags.rsp src/cof.c -Iinclude -odist/cof-wasm.js
