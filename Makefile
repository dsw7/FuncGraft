.PHONY = format compile clean lint test
.DEFAULT_GOAL = compile

format:
	@clang-format -i --verbose --style=file src/*.cpp src/*.hpp

compile: format
	@cmake -S src -B build/prod
	@make --jobs=12 --directory=build/prod install

clean:
	@rm -rfv build

lint:
	@cppcheck src --enable=all

test: export PATH_BIN = $(CURDIR)/build/test/edit
test: format
	@cmake -S src -B build/test -DENABLE_TESTING=ON
	@make --jobs=12 --directory=build/test
	@python3 -m unittest -v tests/test*.py -f
