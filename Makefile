BUILD_DIR = build

clean:
	@rm -rf ${BUILD_DIR}/*

build: clean
	@cmake -S . -B build
	@cmake --build build

test: build
	@cd build && ctest

.PHONY: clean build test
