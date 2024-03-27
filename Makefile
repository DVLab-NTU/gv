all: build

build:
	cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=1
	$(MAKE) -C build -j 8

release:
	cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=Release
	$(MAKE) -C build -j 8

# force macos to use the clang++ installed by brew instead of the default one
# which is outdated
build-clang++:
	cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=1 $(shell which clang++)
	$(MAKE) -C build

# run all tests with current gv binary at the root of the project
# use ./scripts/RUN_TESTS to run tests with specific dofiles
test:
	./scripts/RUN_TEST -v

test-full:
	./scripts/RUN_TEST -v -f
# run clang-format and clang-tidy on the source code
# lint:
# 	./scripts/LINT

debug:
	cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=Debug
	$(MAKE) -C build -j 8

clean:
	cmake --build build --target clean
	rm -rf bin
#	rm -rf build

.PHONY: all build build-clang++ test lint clean 