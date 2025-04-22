#include "../include/platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define MIN_FILE_SIZE 1

sp_file sp_OpenFile(const char* file, sp_file_mode mode, sp_file_flags flags, spf_metadata* meta) {

    int flag = 0;
    if (mode & spf_READ && mode & spf_WRITE) {
        printf("hit\n");
        flag |= O_RDWR;
    } else if (mode & spf_READ) {
        flag |= O_RDONLY;
    } else if (mode & spf_WRITE) {
        flag |= O_WRONLY;
    }

    if (flags & spf_APPEND) {
        flag |= O_APPEND;
    }

    if (flags & spf_TRUNC) {
        flag |= O_APPEND;
    }

    if (flags & spf_CREAT) {
        flag |= O_CREAT;
    }

    int fd = open(file, flag, 0664);
    struct stat stbuf;
    if (fstat(fd, &stbuf)) {
        exit(-1);
    }

    meta->size = stbuf.st_size;
    if (!stbuf.st_size) {
        meta->size = MIN_FILE_SIZE;
        posix_fallocate(fd, 0, meta->size);
    }

    return fd;
}

void sp_CloseFile(sp_file file) {
    close(file);
}


typedef struct spm_mem_head {
    uint64_t size;
    char data[];
} spm_mem_head;


void* sp_MemMapFile(sp_file f, spf_metadata meta, sp_map_flags flags) {
    int prot = 0;
    int type = 0;

    if (flags & spm_READ) {
        prot |= PROT_READ;
    }

    if (flags & spm_WRITE) {
        prot |= PROT_WRITE;
    }

    if (flags & spm_PRIVATE) {
        type |= MAP_PRIVATE;
    } else if (flags & spm_SHARED) {
        type |= MAP_SHARED;
    }

    return mmap(NULL, meta.size, prot, type, f, 0); 
}

void sp_UnMapFile(void* data, spf_metadata meta) {
    munmap(data, meta.size);
}
