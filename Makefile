MAIN=test.cpp
SOURCES=Circuit.cpp Node.cpp Component.cpp Resistor.cpp ISource.cpp VSource.cpp IntegratingComponent.cpp Inductor.cpp Capacitor.cpp Modulator.cpp PWM.cpp Sine.cpp

OBJS=$(SOURCES:%.cpp=build/%.o)

CFLAGS=-O2 -Wall -Wextra -std=c++11 -I/usr/local/include -I/usr/local/include/eigen3

GSL=-lgsl -lgslcblas -lm
LIBS=-L/usr/local/lib $(GSL)

test: $(OBJS) $(MAIN)
	$(CXX) $(CFLAGS) $(LIBS) -o $@ $^

libspice.dylib: $(OBJS)
	$(CXX) -shared $(CFLAGS) $(LIBS) -o $@ $^

build/%.o: %.cpp *.hpp
	@mkdir -p build
	$(CXX) -c $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -f $(OBJS) test libspice.dylib
