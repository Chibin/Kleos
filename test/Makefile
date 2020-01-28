math.o:
	g++ -c ../src/math.cpp

rectangle.o:
	g++ -std=c++14 -c ../src/rectangle.cpp math.o -I../include

array: math.o
	gcc test_array.cpp math.o -l SDL2

hash: math.o
	gcc test_hash.cpp math.o -l SDL2 -o hash

spacial_hash: math.o rectangle.o
	g++ -std=c++14 test_spacial_hash.cpp math.o -I../include -l SDL2 -o spacial_hash_test
