CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2
TARGET = calculator

all: $(TARGET)

$(TARGET): main.o
	$(CXX) $(CXXFLAGS) -o $(TARGET) main.o

main.o: main.cpp BigNumber.hpp BigUnsigned.hpp BigInteger.hpp BigRational.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp

clean:
	rm -f *.o $(TARGET)
