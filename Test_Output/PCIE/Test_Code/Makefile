BUILD_DIR=build
all: configure build

configure:
	mkdir -p $(BUILD_DIR) && cd $(BUILD_DIR) && cmake ..

build:
	cd $(BUILD_DIR) && cmake --build .

run: build
	./$(BUILD_DIR)/pcie_tests

clean:
	rm -rf $(BUILD_DIR)
