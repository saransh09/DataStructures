.PHONY: all build test test-verbose clean configure configure-tsan test-tsan help
# Default number of parallel jobs
JOBS := $(shell sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)
# Build directory
BUILD_DIR := build
BUILD_TSAN_DIR := build-tsan
# Default target
all: build
# Configure CMake (only if needed)
configure:
	@cmake -B $(BUILD_DIR) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
# Configure with ThreadSanitizer
configure-tsan:
	@cmake -B $(BUILD_TSAN_DIR) -DSANITIZER=thread
# Build the project
build: configure
	@cmake --build $(BUILD_DIR) -j$(JOBS)
# Run all tests
test: build
	@./$(BUILD_DIR)/bin/ThreadSafeQueue_tests
# Run tests with verbose output
test-verbose: build
	@./$(BUILD_DIR)/bin/ThreadSafeQueue_tests --success
# Run specific test by name (usage: make test-one NAME="FIFO ordering")
test-one: build
	@./$(BUILD_DIR)/bin/ThreadSafeQueue_tests "$(NAME)"
# Build and run with ThreadSanitizer
test-tsan: configure-tsan
	@cmake --build $(BUILD_TSAN_DIR) -j$(JOBS)
	@./$(BUILD_TSAN_DIR)/bin/ThreadSafeQueue_tests
# Clean build artifacts
clean:
	@rm -rf $(BUILD_DIR) $(BUILD_TSAN_DIR)
# Show available targets
help:
	@echo "Available targets:"
	@echo "  make build        - Build the project"
	@echo "  make test         - Build and run all tests"
	@echo "  make test-verbose - Run tests with verbose output"
	@echo "  make test-one NAME=\"test name\" - Run a specific test"
	@echo "  make test-tsan    - Run tests with ThreadSanitizer"
	@echo "  make clean        - Remove build directories"
	@echo "  make help         - Show this help"
