#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s IMAGE_FILE\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE* f = fopen(argv[1], "wb");
    if (!f) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    const uint8_t boot[] = {
        0x18, 0x0c, 0x00, 0x00,     // jump instruction: jp 0xc
        0x9f, 0x45, 0xa8, 0xc3,     // SimpleFS signature
        0x01, 0x00,                 // SimpleFS version
        0xff, 0xff,                 // Boot file entry
    };

    if (fwrite(boot, sizeof(boot), 1, f) == 0) {
        perror("fwrite");
        exit(EXIT_FAILURE);
    }

    fseek(f, (129088 * 512) - 1, SEEK_SET);
    fputc(0, f);

    fclose(f);
}
