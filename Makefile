CXX = g++
CXXFLAGS = -O2 -std=c++20
TARGET = ssh-route-fix
SRC = ssh-route-fix.cpp
DEPS = helpers.cpp

all: $(TARGET)

$(TARGET): $(SRC) $(DEPS)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)
	sudo chown root:root $(TARGET)
	sudo chmod 4755 $(TARGET)
	sudo cp --preserve=mode,ownership --remove-destination $(TARGET) ~/.local/bin/$(TARGET)

clean:
	sudo rm $(TARGET)
	sudo rm ~/.local/bin/$(TARGET)

.PHONY: all clean
