.PHONY = format compile tidy clean lint compile-test test test-ollama test-slow py
.DEFAULT_GOAL = compile

format:
	@clang-format -i --verbose --style=file \
		src/*.cpp src/*.hpp \
		src/adapters/*.cpp src/adapters/*.hpp \
		src/pipeline/*.cpp src/pipeline/*.hpp \
		src/prompt/*.cpp src/prompt/*.hpp

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
	@python3 -m pytest -vs -k "not ollama" -m "not slow" tests/
	@lcov --quiet --capture --directory=build/test --output-file build/test/coverage.info
	@lcov --quiet --remove build/test/coverage.info "/usr/*" "*/external/*" --output-file build/test/coverage.info
	@genhtml --quiet build/test/coverage.info --output-directory build/test/coverageResults
	@echo "See coverage report at: build/test/coverageResults/index.html"

test-ollama: export PATH_BIN = $(CURDIR)/build/test/edit
test-ollama: format compile-test
	@python3 -m pytest -vs -k "not openai" -m "not slow" tests/
	@lcov --quiet --capture --directory=build/test --output-file build/test/coverage.info
	@lcov --quiet --remove build/test/coverage.info "/usr/*" "*/external/*" --output-file build/test/coverage.info
	@genhtml --quiet build/test/coverage.info --output-directory build/test/coverageResults
	@echo "See coverage report at: build/test/coverageResults/index.html"

test-slow : export PATH_BIN = $(CURDIR)/build/test/edit
test-slow : format compile-test
	@python3 -m pytest -vs -m "slow" tests/

py:
	@black tests/*.py
	@pylint --exit-zero tests/*.py
	@mypy --strict tests/*.py
