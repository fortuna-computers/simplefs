#define FUSE_USE_VERSION 31
#define _GNU_SOURCE

#include <fuse.h>

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static FILE* f = NULL;

static void* sfs_init(struct fuse_conn_info *conn, struct fuse_config *cfg)
{
    (void) conn; (void) cfg;
    return NULL;
}


static void sfs_destroy(void* data)
{
    (void) data;
    fclose(f);
}

static int sfs_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi)
{
    (void) fi;

    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }

    return -ENOENT;
}

static int sfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                     off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags)
{
    (void) offset; (void) fi; (void) flags;

    if (strcmp(path, "/") != 0)
        return -ENOENT;

    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);
    filler(buf, "@boot", NULL, 0, 0);

    return 0;
}

static struct fuse_operations myfs_ops = {
    .init    = sfs_init,
    .destroy = sfs_destroy,
    .getattr = sfs_getattr,
    .readdir = sfs_readdir,
};

int main(int argc, char* argv[])
{
    int i;
 
    // get the device or image filename from arguments
    for (i = 1; i < argc && argv[i][0] == '-'; i++);
    if (i < argc) {
        char* devfile = realpath(argv[i], NULL);
        memcpy(&argv[i], &argv[i+1], (argc-i) * sizeof(argv[0]));
        argc--;

        f = fopen(devfile, "rwb");
        if (!f) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "Expected image or device.");
        exit(EXIT_FAILURE);
    }

    return fuse_main(argc, argv, &myfs_ops, NULL);
}
