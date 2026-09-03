CXX = g++
CXXFLAGS = -Os -std=c++20 -fno-exceptions -fno-rtti \
           -ffunction-sections -fdata-sections -fno-asynchronous-unwind-tables
LDFLAGS = -s -Wl,--gc-sections -Wl,--build-id=none
TARGET = ssh-route-fix
INSTALL_TO = ~/.local/bin/$(TARGET)
COMPLETIONS_DIR = $(HOME)/.local/share/bash-completion/completions
COMPLETIONS_FILE = $(COMPLETIONS_DIR)/$(TARGET)
SRC = ssh-route-fix.cpp
DEPS = helpers.cpp

all: $(TARGET)

$(TARGET): $(SRC) $(DEPS)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

install: $(TARGET)
	sudo cp --remove-destination $(TARGET) $(INSTALL_TO)
	sudo chown root:root $(INSTALL_TO)
	sudo chmod 4755 $(INSTALL_TO)
	mkdir -p $(COMPLETIONS_DIR)
	./$(TARGET) --print-completion > $(COMPLETIONS_FILE)

uninstall:
	sudo rm $(INSTALL_TO)
	rm -f $(COMPLETIONS_FILE)

clean:
	rm $(TARGET)

.PHONY: all clean
