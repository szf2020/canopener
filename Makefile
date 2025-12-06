.PHONY: test
test:
	g++ -obin/test -Iinclude src/*.c* test/test-main.cpp
	./bin/test
