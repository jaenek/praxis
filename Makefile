.PHONY: all build configure run

all: build

out:
	mkdir out

configure: out
	cd $^ && cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=1 && cp compile_commands.json ../

build: out
	cd $^ && make

run:
	./out/praxis
