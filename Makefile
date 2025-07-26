.PHONY = format compile tidy clean lint test coverage
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
	@cmake -S src -B build/test -DENABLE_TESTING=ON
	@make --jobs=12 --directory=build/test
	@python3 -m unittest -v tests/test*.py -f

coverage: export PATH_BIN = $(CURDIR)/build/coverage/edit
coverage: format
	@cmake -S src -B build/coverage -DENABLE_TESTING=ON -DENABLE_COVERAGE=ON
	@make --jobs=12 --directory=build/coverage
	@python3 -m unittest -v tests/test*.py -f
	@lcov --capture --directory=build/coverage --output-file build/coverage/coverage.info
	@genhtml build/coverage/coverage.info --output-directory build/coverage/coverageResults
	@echo "See coverage report at: build/coverage/coverageResults/index.html"
