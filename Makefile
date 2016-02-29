all: pathhack.so

pathhack.so: pathhack.c paths
	sed 's/.*/PATH(&)/' < paths > paths_gen.h
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -fPIC pathhack.c -o pathhack.so -ldl -Wall -Wextra -std=c99
