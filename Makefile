all: map_test

map_test: insertion_ordered_map.h insertion_ordered_map_example.cc
	@echo "Kompiluję insertion_ordered_map_example.cc"
	g++ -Wall -Wextra -std=c++17 -O2 insertion_ordered_map_example.cc -o map_test
