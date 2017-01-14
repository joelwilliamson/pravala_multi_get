
CXXFLAGS += -I. -g  -lboost_system -lboost_program_options

all: build/client test

build/ci_string.o: ci_string.hpp ci_string.cpp
	$(CXX) $(CXXFLAGS) ci_string.cpp -c -o build/ci_string.o

build/message.o: message.hpp message.cpp
	$(CXX) $(CXXFLAGS) message.cpp -c -o build/message.o

build/message_test.o: message.hpp test/message_test.cpp
	$(CXX) $(CXXFLAGS) test/message_test.cpp -c -o build/message_test.o

build/network.o: network.cpp network.hpp message.hpp
	$(CXX) $(CXXFLAGS) network.cpp -c -o build/network.o

build/client: client.cpp build/network.o build/message.o build/ci_string.o
	$(CXX) $(CXXFLAGS)  -pthread client.cpp build/network.o build/message.o build/ci_string.o -o build/client

build/test_main.o: test/test_main.cpp
	$(CXX) $(CXXFLAGS) test/test_main.cpp -c -o build/test_main.o

test: build/message.o build/message_test.o build/test_main.o build/ci_string.o
	$(CXX) $(CXXFLAGS) build/test_main.o build/message.o build/message_test.o build/ci_string.o -o build/test
	build/test

clean:
	rm build/*

.PHONY: test clean all


