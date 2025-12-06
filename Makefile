.PHONY: test
test:
	g++ -obin/test -Iinclude src/*.cpp src/cof.c test/test-main.cpp
	./bin/test
