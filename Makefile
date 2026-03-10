# Ensure macOS SDK is found by all subprojects (fixes standard header not found on some systems)
CMAKE_OSX_SYSROOT ?= $(shell xcrun --sdk macosx --show-sdk-path 2>/dev/null)

# On macOS, prefer Homebrew bison (3.x) for vcd-parser; system bison 2.x is too old
export PATH := $(shell brew --prefix 2>/dev/null)/opt/bison/bin:$(shell brew --prefix 2>/dev/null)/bin:$(PATH)

# On Apple Silicon, build for arm64 to avoid x86_64/arm64 link mismatch
CMAKE_ARCH := $(if $(filter arm64,$(shell uname -m 2>/dev/null)),-DCMAKE_OSX_ARCHITECTURES=arm64,)

all: build

build:
	cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
		$(if $(CMAKE_OSX_SYSROOT),-DCMAKE_OSX_SYSROOT=$(CMAKE_OSX_SYSROOT)) \
		$(CMAKE_ARCH)
	$(MAKE) -C build -j 8 --no-print-directory

release:
	cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=Release \
		$(if $(CMAKE_OSX_SYSROOT),-DCMAKE_OSX_SYSROOT=$(CMAKE_OSX_SYSROOT)) \
		$(CMAKE_ARCH)
	$(MAKE) -C build -j 8 --no-print-directory

# force macos to use the clang++ installed by brew instead of the default one
# which is outdated
# build-clang++:
# 	cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=1 $(shell which clang++)
# 	$(MAKE) -C build

# run all tests with current gv binary at the root of the project
# use ./scripts/RUN_TESTS to run tests with specific dofiles
test:
	./scripts/RUN_TEST -v

test-full:
	./scripts/RUN_TEST -v -f

test-update:
	./scripts/RUN_TEST -u

debug:
	cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=Debug \
		$(if $(CMAKE_OSX_SYSROOT),-DCMAKE_OSX_SYSROOT=$(CMAKE_OSX_SYSROOT)) \
		$(CMAKE_ARCH)
	$(MAKE) -C build -j 8

clean:
	cmake --build build --target clean
	rm -rf bin
#	rm -rf build

.PHONY: all build build-clang++ test lint clean 
