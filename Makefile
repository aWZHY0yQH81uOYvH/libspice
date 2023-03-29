SOURCES=test.cpp Circuit.cpp Node.cpp Component.cpp Resistor.cpp ISource.cpp VSource.cpp

OBJS=$(SOURCES:%.cpp=build/%.o)

CFLAGS=-O2 -Wall -Wextra -std=c++11 -I/usr/local/include -I/usr/local/include/eigen3

GSL=-lgsl -lgslcblas -lm
LIBS=-L/usr/local/lib $(GSL)

test: $(OBJS)
	$(CXX) $(CFLAGS) $(LIBS) -o $@ $^

build/%.o: %.cpp *.hpp
	$(CXX) -c $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -f $(OBJS)
