# NEXUS Programming Language Makefile

# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -march=native
DEBUG_FLAGS = -g -DDEBUG -O0
RELEASE_FLAGS = -DNDEBUG -O3 -flto
PROFILE_FLAGS = -pg -fprofile-arcs -ftest-coverage

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin
TEST_DIR = tests
DOC_DIR = docs
EXAMPLES_DIR = examples

# Source files
# Fixed - since you're already in Src directory
SOURCES = $(wildcard *.cpp) \
          $(wildcard ml/*.cpp) \
          $(wildcard utils/*.cpp)

# Object files
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Test sources
TEST_SOURCES = $(wildcard $(TEST_DIR)/unit/*.cpp) \
               $(wildcard $(TEST_DIR)/integration/*.cpp)
TEST_OBJECTS = $(TEST_SOURCES:$(TEST_DIR)/%.cpp=$(BUILD_DIR)/test_%.o)

# Headers
HEADERS = $(wildcard $(SRC_DIR)/*.h) \
          $(wildcard $(SRC_DIR)/ml/*.h) \
          $(wildcard $(SRC_DIR)/utils/*.h) \
          $(wildcard $(INCLUDE_DIR)/nexus/*.h)

# Target executable
TARGET = $(BIN_DIR)/nexus
TEST_TARGET = $(BIN_DIR)/nexus_tests

# Libraries
LIBS = -lm -lpthread
GPU_LIBS = -lcuda -lcublas -lcudnn
BLAS_LIBS = -lblas -llapack

# Include paths
INCLUDES = -I$(INCLUDE_DIR) -I$(SRC_DIR)

# Platform-specific settings
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    LIBS += -ldl
    PLATFORM = linux
endif
ifeq ($(UNAME_S),Darwin)
    LIBS += -framework Accelerate
    PLATFORM = macos
endif
ifeq ($(OS),Windows_NT)
    TARGET = $(BIN_DIR)/nexus.exe
    PLATFORM = windows
endif

# Build configurations
.PHONY: all clean debug release profile test install uninstall docs examples

# Default target
all: release

# Release build
release: CXXFLAGS += $(RELEASE_FLAGS)
release: $(TARGET)

# Debug build
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: $(TARGET)

# Profile build
profile: CXXFLAGS += $(PROFILE_FLAGS)
profile: LIBS += -lgcov
profile: $(TARGET)

# GPU build
gpu: CXXFLAGS += -DUSE_GPU
gpu: LIBS += $(GPU_LIBS)
gpu: $(TARGET)

# BLAS build
blas: CXXFLAGS += -DUSE_BLAS
blas: LIBS += $(BLAS_LIBS)
blas: $(TARGET)

# Main target
$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(OBJECTS) -o $@ $(LIBS)
	@echo "Built NEXUS interpreter: $@"

# Object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS) | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Create directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)/ml $(BUILD_DIR)/utils

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Tests
test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJECTS) $(filter-out $(BUILD_DIR)/main.o,$(OBJECTS)) | $(BIN_DIR)
	$(CXX) $^ -o $@ $(LIBS) -lgtest -lgtest_main
	@echo "Built test executable: $@"

$(BUILD_DIR)/test_%.o: $(TEST_DIR)/%.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Unit tests only
unit-tests: CXXFLAGS += $(DEBUG_FLAGS)
unit-tests:
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(TEST_DIR)/unit/*.cpp \
	$(filter-out $(BUILD_DIR)/main.o,$(OBJECTS)) -o $(BIN_DIR)/unit_tests $(LIBS) -lgtest -lgtest_main
	./$(BIN_DIR)/unit_tests

# Integration tests
integration-tests: $(TARGET)
	@echo "Running integration tests..."
	@for script in $(EXAMPLES_DIR)/*.nx; do \
		echo "Testing $$script"; \
		./$(TARGET) $$script; \
	done

# Benchmarks
benchmark: profile
	@echo "Running benchmarks..."
	./$(BIN_DIR)/nexus $(EXAMPLES_DIR)/benchmarks/performance_test.nx
	gprof ./$(BIN_DIR)/nexus gmon.out > benchmark_report.txt

# Memory tests
memory-test: debug
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET) $(EXAMPLES_DIR)/basic/hello_world.nx

# Static analysis
static-analysis:
	cppcheck --enable=all --std=c++17 $(SRC_DIR)/ $(INCLUDE_DIR)/
	clang-tidy $(SOURCES) -- $(INCLUDES)

# Code coverage
coverage: CXXFLAGS += --coverage
coverage: LIBS += -lgcov
coverage: test
	gcov $(SOURCES)
	lcov --capture --directory $(BUILD_DIR) --output-file coverage.info
	genhtml coverage.info --output-directory coverage_html

# Documentation
docs:
	doxygen Doxyfile
	@echo "Documentation generated in $(DOC_DIR)/html"

# Install system-wide
install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/
	sudo mkdir -p /usr/local/lib/nexus
	sudo cp -r stdlib/* /usr/local/lib/nexus/
	sudo cp -r $(INCLUDE_DIR)/* /usr/local/include/
	@echo "NEXUS installed to /usr/local/bin/nexus"

# Uninstall
uninstall:
	sudo rm -f /usr/local/bin/nexus
	sudo rm -rf /usr/local/lib/nexus
	sudo rm -rf /usr/local/include/nexus
	@echo "NEXUS uninstalled"

# Package for distribution
package: release
	mkdir -p nexus-$(VERSION)-$(PLATFORM)
	cp $(TARGET) nexus-$(VERSION)-$(PLATFORM)/
	cp -r stdlib nexus-$(VERSION)-$(PLATFORM)/
	cp -r examples nexus-$(VERSION)-$(PLATFORM)/
	cp -r docs nexus-$(VERSION)-$(PLATFORM)/
	cp README.md LICENSE nexus-$(VERSION)-$(PLATFORM)/
	tar -czf nexus-$(VERSION)-$(PLATFORM).tar.gz nexus-$(VERSION)-$(PLATFORM)/
	rm -rf nexus-$(VERSION)-$(PLATFORM)

# Docker build
docker:
	docker build -t nexus-lang .
	docker tag nexus-lang nexus-lang:$(VERSION)

# Docker GPU build
docker-gpu:
	docker build -f docker/gpu.Dockerfile -t nexus-lang:gpu .

# Examples
examples: $(TARGET)
	@echo "Running example programs..."
	@for category in basic ml advanced; do \
		echo "Running $$category examples:"; \
		for script in $(EXAMPLES_DIR)/$$category/*.nx; do \
			echo "  - $$(basename $$script)"; \
			./$(TARGET) $$script; \
		done; \
	done

# Performance testing
perf: $(TARGET)
	@echo "Performance testing..."
	time ./$(TARGET) $(EXAMPLES_DIR)/benchmarks/large_neural_network.nx
	time ./$(TARGET) $(EXAMPLES_DIR)/benchmarks/matrix_operations.nx

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
	rm -f *.gcov *.gcda *.gcno coverage.info
	rm -rf coverage_html
	rm -f benchmark_report.txt gmon.out
	@echo "Cleaned build artifacts"

# Clean everything including docs
clean-all: clean
	rm -rf $(DOC_DIR)/html $(DOC_DIR)/latex
	rm -f *.tar.gz
	rm -rf nexus-*-*

# Format code
format:
	clang-format -i $(SOURCES) $(HEADERS)
	@echo "Code formatted"

# Lint code
lint:
	cpplint $(SOURCES) $(HEADERS)

# Version information
version:
	@echo "NEXUS Programming Language"
	@echo "Version: $(shell grep NEXUS_VERSION $(INCLUDE_DIR)/nexus/version.h | cut -d'"' -f2)"
	@echo "Platform: $(PLATFORM)"
	@echo "Compiler: $(CXX)"

# Help
help:
	@echo "NEXUS Programming Language Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  all          - Build release version (default)"
	@echo "  debug        - Build debug version"
	@echo "  release      - Build optimized release version"
	@echo "  profile      - Build with profiling enabled"
	@echo "  gpu          - Build with GPU support"
	@echo "  blas         - Build with BLAS acceleration"
	@echo ""
	@echo "Testing:"
	@echo "  test         - Run all tests"
	@echo "  unit-tests   - Run unit tests only"
	@echo "  integration-tests - Run integration tests"
	@echo "  benchmark    - Run performance benchmarks"
	@echo "  memory-test  - Run memory leak detection"
	@echo "  coverage     - Generate code coverage report"
	@echo ""
	@echo "Quality:"
	@echo "  static-analysis - Run static code analysis"
	@echo "  format       - Format code with clang-format"
	@echo "  lint         - Lint code with cpplint"
	@echo ""
	@echo "Documentation:"
	@echo "  docs         - Generate documentation"
	@echo ""
	@echo "Distribution:"
	@echo "  install      - Install system-wide"
	@echo "  uninstall    - Remove system installation"
	@echo "  package      - Create distribution package"
	@echo "  docker       - Build Docker image"
	@echo "  docker-gpu   - Build GPU Docker image"
	@echo ""
	@echo "Utilities:"
	@echo "  examples     - Run example programs"
	@echo "  perf         - Performance testing"
	@echo "  clean        - Clean build artifacts"
	@echo "  clean-all    - Clean everything"
	@echo "  version      - Show version information"
	@echo "  help         - Show this help"

# Dependencies
-include $(OBJECTS:.o=.d)

# Automatic dependency generation
$(BUILD_DIR)/%.d: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -MM -MT $(@:.d=.o) $< > $@