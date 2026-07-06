CC = clang

3d_numbers: 3d_numbers.c
	$(CC) -g -o 3d_numbers 3d_numbers.c -lraylib -lGL -lm -lpthread -ldl -lrt
