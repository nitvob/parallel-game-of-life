CXX = g++
MPIXX  = mpicxx
CFLAGS = -O2

all: life-nonblocking serial

life-nonblocking: life-nonblocking.cpp
	$(MPIXX) $(CFLAGS) -o $@ $<

serial: serial.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f life-nonblocking serial