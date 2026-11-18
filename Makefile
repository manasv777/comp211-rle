CC = cc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Werror -O2
CPPFLAGS = -Isrc

.PHONY: all demo test sanitize results clean
all: build/rle_demo

build:
	mkdir -p build

build/rle_demo: src/demo.c src/rle.c src/rle.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) src/demo.c src/rle.c -o $@

build/test_rle: tests/test_rle.c src/rle.c src/rle.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_rle.c src/rle.c -o $@

demo: build/rle_demo
	./build/rle_demo

test: build/test_rle
	./build/test_rle

sanitize: | build
	$(CC) $(CPPFLAGS) -std=c11 -Wall -Wextra -Wpedantic -Werror -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer tests/test_rle.c src/rle.c -o build/test_rle_sanitize
	./build/test_rle_sanitize

results: build/rle_demo
	mkdir -p results
	./build/rle_demo --csv > results/experiments.csv
	./build/rle_demo > results/demo-output.txt

clean:
	rm -rf build
