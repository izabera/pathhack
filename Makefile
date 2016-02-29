all:
	sed 's/.*/PATH(&)/' < paths > paths_gen.h
	gcc -shared -fPIC pathhack.c -o pathhack.so -ldl -Wall -Wextra -std=c99
