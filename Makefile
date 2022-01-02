CPPFLAGS = -std=c11 -Wall -Wextra -ggdb -O0

all: mkfs.simplefs mount.simplefs

mkfs.simplefs: mkfs.o
	$(CC) $^ -o $@

mount.simplefs: CPPFLAGS += -D_FILE_OFFSET_BITS=64 
mount.simplefs: fuse.o
	$(CC) $^ -o $@ `pkg-config --cflags --libs fuse3`

clean:
	rm -f *.o mkfs.simplefs mount.simplefs
.PHONY: clean
