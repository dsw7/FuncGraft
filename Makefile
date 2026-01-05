.PHONY = format compile tidy clean lint test
.DEFAULT_GOAL = compile

format:
	@clang-format -i --verbose --style=file src/*.cpp src/*.hpp

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

test: export PATH_BIN = $(CURDIR)/build/test/edit
test: format
	@cmake -S src -B build/test -DENABLE_TESTING=ON -DENABLE_COVERAGE=ON
	@make --jobs=12 --directory=build/test
	@python3 -m pytest -vs tests/
	@lcov --capture --directory=build/test --output-file build/test/coverage.info
	@lcov --remove build/test/coverage.info "/usr/*" "*/external/*" --output-file build/test/coverage.info
	@genhtml build/test/coverage.info --output-directory build/test/coverageResults
	@echo "See coverage report at: build/test/coverageResults/index.html"
