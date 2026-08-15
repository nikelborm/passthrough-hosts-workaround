CXX = g++
CXXFLAGS = -O2 -std=c++20
TARGET = ssh-route-fix
SRC = ssh_passthrough.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean
