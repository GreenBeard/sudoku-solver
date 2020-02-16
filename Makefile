NOECHO=@

c_files=main sudoku_grid sudoku_solver
cxx_files=sudoku_generator
build_objects=$(patsubst %, build/%.o, $(c_files)) \
  $(patsubst %, build/%.o, $(cxx_files))
c_src_files=$(patsubst %, src/%.c, $(c_files))
cxx_src_files=$(patsubst %, src/%.c, $(cxx_files))
LIBS=-lpthread -lqqwing

TARGET=sudoku_solver

help:
	$(NOECHO)echo "Commands:"
	$(NOECHO)echo "  \"make build\" - builds the project (not smart yet, clean recommended)"
	$(NOECHO)echo "  \"make clean\" - cleans the build files"

build/%.o: src/%.c build_dir
	$(CC) -O2 -std=c99 -fasm -fno-omit-frame-pointer -g -Wvla -Wall -Wwrite-strings -pedantic -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=700 -c $< -o $@

build/%.o: src/%.cpp build_dir
	$(CXX) -O2 -std=c++98 -fasm -fno-omit-frame-pointer -g -Wvla -Wall -Wwrite-strings -pedantic -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=700 -c $< -o $@

bin/$(TARGET): bin_dir
	$(CXX) $(build_objects) -o $@ $(LIBS)

build: $(build_objects) bin/$(TARGET)
	$(NOECHO)echo Done!

clean:
	rm -rf ./build/
	rm -rf ./bin/

%_dir:
	$(NOECHO)mkdir -p $*

.PHONY: build clean
