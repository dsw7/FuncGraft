.PHONY = help format compile clean lint test
.DEFAULT_GOAL = help

define HELP_LIST_TARGETS
To format code:
    $$ make format
To compile binary:
    $$ make compile
To remove build directory:
    $$ make clean
To run cppcheck linter:
    $$ make lint
To run unit tests:
    $$ make test
endef

export HELP_LIST_TARGETS

help:
	@echo "$$HELP_LIST_TARGETS"

format:
	@clang-format -i --verbose --style=file src/*.cpp src/*.hpp

compile: format
	@cmake -S src -B build/prod
	@make --jobs=12 --directory=build/prod install

clean:
	@rm -rfv build

lint:
	@cppcheck src --enable=all

test: export PATH_BIN = $(CURDIR)/build/test/gpe
test: format
	@cmake -S src -B build/test -DENABLE_TESTING=ON
	@make --jobs=12 --directory=build/test
	@python3 -m unittest -v tests/test*.py -f
