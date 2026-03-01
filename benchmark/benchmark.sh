#!/bin/bash

clear

echo "Compiling..."
make fclean
make
make clean

benchmark_hotrace() {
	echo "=================================="
	echo "Testing for $1 entries and $2 tests"
	python3.12 benchmark/file_generator.py $1 $2
	time ./hotrace <input.htr >/dev/null
	echo -e "==================================\n"
}

benchmark_hotrace 1000000 500000
