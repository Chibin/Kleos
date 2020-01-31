math.o:
	clang -c ../src/math.cpp

rectangle.o: math.o
	clang -std=c++14 -c ../src/rectangle.cpp math.o -I../include -l SDL2 -lm

hash_set.o: math.o rectangle.o
	clang -std=c++14 -c ../src/hash_set.cpp math.o rectangle.o -I../include -l SDL2 -lm

array: math.o
	gcc test_array.cpp math.o -l SDL2

hash: math.o
	gcc test_hash.cpp math.o -l SDL2 -o hash

hash_set:
	clang -std=c++14 hash_set.cpp -I../include -l SDL2 -lm

spatial_hash: math.o rectangle.o hash_set.o
	clang -std=c++14 math.o rectangle.o hash_set.o test_spatial_hash.cpp \
		-I../include -l SDL2 -lm -o spatial_hash_test

hash_set: math.o rectangle.o
	clang -std=c++14 math.o rectangle.o test_hash_set.cpp -I../include -l SDL2 -lm -o hash_set_test

clean:
	rm -rf *.o
