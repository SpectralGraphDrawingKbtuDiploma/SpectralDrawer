CXX = g++
CXXFLAGS = -O2 -std=c++17 -Iinclude -Iinclude/spectra/include -Iinclude/eigen

SRC = src/main.cpp
OUT = eigen_solver

all: $(OUT)

$(OUT): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)
