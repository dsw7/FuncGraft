.PHONY = format compile tidy clean lint compile-test test test-ollama py
.DEFAULT_GOAL = compile

format:
	@clang-format -i --verbose --style=file \
		src/*.cpp src/*.hpp \
		src/prompt/*.cpp src/prompt/*.hpp \
		src/completion/*.cpp src/completion/*.hpp \
		src/pipeline/*.cpp src/pipeline/*.hpp

compile: format
	@cmake -S src -B build/prod
	@make --jobs=12 --directory=build/prod install

tidy:
	@cmake -S src -B build/prod  # generate build files so clang-tidy can find compile_commands.json
	@clang-tidy -p build/prod src/*.cpp src/*.hpp

clean:
	@rm -rfv build

lint:
	@cppcheck src --enable=all

compile-test:
	@cmake -S src -B build/test -DENABLE_TESTING=ON -DENABLE_COVERAGE=ON
	@make --jobs=12 --directory=build/test

test: export PATH_BIN = $(CURDIR)/build/test/edit
test: format compile-test
	@python3 -m pytest -vs -m "test_misc or test_openai" tests/
	@lcov --capture --directory=build/test --output-file build/test/coverage.info
	@lcov --remove build/test/coverage.info "/usr/*" "*/external/*" --output-file build/test/coverage.info
	@genhtml build/test/coverage.info --output-directory build/test/coverageResults
	@echo "See coverage report at: build/test/coverageResults/index.html"

test-ollama: export PATH_BIN = $(CURDIR)/build/test/edit
test-ollama: format compile-test
	@python3 -m pytest -vs -m "test_misc or test_ollama" tests/
	@lcov --capture --directory=build/test --output-file build/test/coverage.info
	@lcov --remove build/test/coverage.info "/usr/*" "*/external/*" --output-file build/test/coverage.info
	@genhtml build/test/coverage.info --output-directory build/test/coverageResults
	@echo "See coverage report at: build/test/coverageResults/index.html"

py:
	@black tests/*.py
	@pylint --exit-zero tests/*.py
	@mypy --strict tests/*.py
