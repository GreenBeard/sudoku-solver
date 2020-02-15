NOECHO=@

c_files=main sudoku_grid sudoku_solver
build_objects=$(patsubst %, build/%.o, $(c_files))
src_files=$(patsubst %, src/%.c, $(c_files))
LIBS=

TARGET=sudoku_solver

help:
	$(NOECHO)echo "Commands:"
	$(NOECHO)echo "  \"make build\" - builds the project (not smart yet, clean recommended)"
	$(NOECHO)echo "  \"make clean\" - cleans the build files"

build/%.o: src/%.c build_dir
	$(CC) -O0 -std=c99 -fasm -g -Wvla -Wall -Wwrite-strings -pedantic -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=700 -c $< -o $@

bin/$(TARGET): bin_dir
	$(CC) $(build_objects) -o $@ $(LIBS)

build: $(build_objects) bin/$(TARGET)
	$(NOECHO)echo Done!

clean:
	rm -rf ./build/
	rm -rf ./bin/

%_dir:
	$(NOECHO)mkdir -p $*

.PHONY: build clean
