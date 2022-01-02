CPPFLAGS = -std=c11 -Wall -Wextra -ggdb -O0

all: mkfs.simplefs

mkfs.simplefs: mkfs.o
	$(CC) $^ -o $@

clean:
	rm -f *.o mkfs.simplefs mount.simplefs
.PHONY: clean
