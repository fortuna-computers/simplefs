#define _GNU_SOURCE

#include <libgen.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

static void argument_error(const char* program)
{
    fprintf(stderr, "Usage: %s [-b BOOT_FILE] IMAGE_FILE [FILES_TO_ADD...]\n", program);
    exit(EXIT_FAILURE);
}

static FILE* create_image_file(const char* image_file, const char* boot_file)
{
    FILE* f = fopen(image_file, "wb");
    if (!f) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    const uint8_t boot[] = {
        0x18, 0x0c, 0x00, 0x00,     // jump instruction: jp 0xc
        0x9f, 0x45, 0xa8, 0xc3,     // SimpleFS signature
        0x01, 0x00,                 // SimpleFS version
    };

    if (fwrite(boot, sizeof(boot), 1, f) == 0) {
        perror("fwrite");
        exit(EXIT_FAILURE);
    }

    fseek(f, (129088 * 512) - 1, SEEK_SET);
    fputc(0, f);

    if (boot_file) {
        const size_t max_boot_size = 512 - 0xc;

        // read file
        uint8_t buf[max_boot_size];
        FILE* fr = fopen(boot_file, "r");
        if (!fr) { perror("fopen"); exit(EXIT_FAILURE); }
        fseek(fr, 0, SEEK_END);
        size_t boot_sz;
        if ((boot_sz = ftell(fr)) > max_boot_size) { fprintf(stderr, "Boot file too large.\n"); exit(EXIT_FAILURE); }
        fseek(fr, 0, SEEK_SET);
        if (fread(buf, boot_sz, 1, fr) != 1) { perror("fread"); exit(EXIT_FAILURE); }
        fclose(fr);

        // write file
        if (fseek(f, 0xc, SEEK_SET) < 0) { perror("fseek"); exit(EXIT_FAILURE); }
        if (fwrite(buf, boot_sz, 1, f) != 1) { perror("fwrite"); exit(EXIT_FAILURE); }
    }

    return f;
}

static void add_file(size_t entry_nr, FILE* f, const char* filename)
{
    char* filename2 = strdup(filename);
    char* fname = basename(filename2);

    if (strlen(fname) > 21) {
        fprintf(stderr, "File name too large: %s", fname);
        exit(EXIT_FAILURE);
    }

    struct stat s;
    if (stat(filename, &s) < 0) { perror("stat"); exit(EXIT_FAILURE); }

    if (s.st_size > (64 * 1024)) {
        fprintf(stderr, "File too large (max 64 kB): %s", fname);
        exit(EXIT_FAILURE);
    }

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    // add file entry
    uint8_t entry[32] = {
        0x0,                        // file status
        0x0,                        // reserved
        s.st_size & 0xff,           // file size
        (s.st_size >> 8) & 0xff,
        (s.st_size >> 16) & 0xff,
        (s.st_size >> 24) & 0xff,
        ((tm.tm_year + 1900) >> 8) & 0xff,   // time
        (tm.tm_year + 1900) & 0xff,
        tm.tm_mon,
        tm.tm_mday
    };
    memset(&entry[0xa], 0, 32 - 0xa);
    strcpy((char *) &entry[0xa], filename);
    if (fseek(f, 512 + (entry_nr * 32), SEEK_SET) < 0) { perror("fseek"); exit(EXIT_FAILURE); }
    if (fwrite(entry, 32, 1, f) == 0) { perror("fwrite"); exit(EXIT_FAILURE); }

    // read file
    uint8_t buf[s.st_size];
    FILE* fr = fopen(filename, "r");
    if (!fr) { perror("fopen"); exit(EXIT_FAILURE); }
    if (fread(buf, s.st_size, 1, fr) != 1) { perror("fread"); exit(EXIT_FAILURE); }
    fclose(fr);

    // write file
    if (fseek(f, ((entry_nr * 128) + 64) * 512, SEEK_SET) < 0) { perror("fseek"); exit(EXIT_FAILURE); }
    if (fwrite(buf, s.st_size, 1, f) != 1) { perror("fwrite"); exit(EXIT_FAILURE); }

    free(filename2);
}

int main(int argc, char* argv[])
{
    char* boot_file = NULL;
    size_t entry_nr = 0;

    FILE* f = NULL;
    for (int i = 1; i < argc; ++i) {

        // boot file argument
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], "-b") == 0) {
                boot_file = argv[i+1];
                ++i;
                continue;
            } else {
                argument_error(argv[0]);
            }

        // image file argument
        } else if (!f) {
            f = create_image_file(argv[i], boot_file);

        // add file
        } else {
            add_file(entry_nr, f, argv[i]);
        }
    }

    if (!f)
        argument_error(argv[0]);
    else
        fclose(f);
}
