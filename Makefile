SOURCES=test.cpp Circuit.cpp Node.cpp Component.cpp Resistor.cpp ISource.cpp VSource.cpp IntegratingComponent.cpp Inductor.cpp Capacitor.cpp Modulator.cpp PWM.cpp Sine.cpp

OBJS=$(SOURCES:%.cpp=build/%.o)

CFLAGS=-O2 -Wall -Wextra -std=c++11 -I/usr/local/include -I/usr/local/include/eigen3

GSL=-lgsl -lgslcblas -lm
MATPLOT=-lmatplot
LIBS=-L/usr/local/lib $(GSL) $(MATPLOT)

test: $(OBJS)
	$(CXX) $(CFLAGS) $(LIBS) -o $@ $^

build/%.o: %.cpp *.hpp
	$(CXX) -c $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -f $(OBJS)
