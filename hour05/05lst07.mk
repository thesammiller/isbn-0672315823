CC = gcc
STD = _GNU_SOURCE

.c.o:
	$(CC) -c -Wall $(CFLAGS) -D$(STD) $< -o $@

