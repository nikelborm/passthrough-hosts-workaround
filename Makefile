CXX = g++
CXXFLAGS = -O2 -std=c++20
TARGET = ssh-route-fix
INSTALL_TO = ~/.local/bin/$(TARGET)
SRC = ssh-route-fix.cpp
DEPS = helpers.cpp

all: $(TARGET)

$(TARGET): $(SRC) $(DEPS)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

install: $(TARGET)
	sudo cp --remove-destination $(TARGET) $(INSTALL_TO)
	sudo chown root:root $(INSTALL_TO)
	sudo chmod 4755 $(INSTALL_TO)

uninstall:
	sudo rm $(INSTALL_TO)

clean:
	rm $(TARGET)

.PHONY: all clean
