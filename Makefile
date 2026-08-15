CXX = g++
CXXFLAGS = -O2 -std=c++20
TARGET = ssh-route-fix
SRC = ssh-route-fix.cpp
DEPS = helpers.cpp

all: $(TARGET)

$(TARGET): $(SRC) $(DEPS)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean
