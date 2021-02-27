override FLAGS += -DCMAKE_EXPORT_COMPILE_COMMANDS=1
.PHONY: all build configure run

all: build

out:
	mkdir out

configure: out
	cd $^ && cmake .. $(FLAGS) && cp compile_commands.json ../

build: out
	cd $^ && make
