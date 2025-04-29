#include "../include/platform.h"
#include <memoryapi.h>

#ifdef _MSC_VER

#define WIN32_LEAN_AND_MEAN
#define _WINUSER_
#define _WINGDI_
#define _IMM_
#define _WINCON_
#include <windows.h>
#include <processthreadsapi.h>
#include <timezoneapi.h>
#include <minwinbase.h>
#include <fileapi.h>
#include <errhandlingapi.h>
#include <winerror.h>
#include <winnt.h>
#include <minwindef.h>
#include <handleapi.h>
#include <winbase.h>

#define MIN_FILE_SIZE 1

sp_file sp_OpenFile(const char* file, sp_file_mode mode, sp_file_flags flags, spf_metadata* meta) {
    DWORD access = 0; 
    DWORD create = OPEN_EXISTING;

    if (mode & spf_READ) {
        access |= GENERIC_READ;
    }

    if (mode & spf_WRITE) {
        access |= GENERIC_WRITE;
    }

    if (mode & spf_CREAT && mode & spf_TRUNC) {
        create = CREATE_ALWAYS;
    } else if (mode & spf_CREAT && mode & spf_APPEND) {
        create = OPEN_ALWAYS;
    } else if (mode & spf_CREAT) {
        create = CREATE_NEW;
    } else if (mode & spf_TRUNC) {
        create = TRUNCATE_EXISTING;
    } else if (mode & spf_APPEND) {
        create = OPEN_EXISTING;
    }

    HANDLE f = CreateFile(file, access, 0, NULL, create, FILE_ATTRIBUTE_NORMAL, NULL); 

    LARGE_INTEGER size;
    GetFileSizeEx(f, &size);
    meta->size = size.QuadPart;

    return f;
}

void sp_ResizeFile(sp_file file, spf_metadata* meta, uint64_t newsize) {
    LARGE_INTEGER move; 
    move.QuadPart = newsize;
    SetFilePointerEx(file, move, NULL, FILE_BEGIN);
    SetEndOfFile(file);
    move.QuadPart = 0;
    SetFilePointerEx(file, move, NULL, FILE_BEGIN);
}

void sp_CloseFile(sp_file file) {
    CloseHandle(file);
}

void* sp_MemMapFile(sp_file f, spf_metadata meta, sp_map_flags flags) {
    DWORD access = 0;

    if (flags & spm_READ && flags & spm_WRITE) {
        access = FILE_MAP_ALL_ACCESS;
    } else if (flags & spm_READ) {
       access = FILE_MAP_READ;
    } else if (flags & spm_WRITE) {
       access = FILE_MAP_WRITE;
    }

    if (!(flags & spm_SHARED)) {
        access |= FILE_MAP_COPY;
    }

    LARGE_INTEGER size;
    size.QuadPart = meta.size;
    meta.private = CreateFileMapping(f, NULL, PAGE_READWRITE, size.HighPart, size.LowPart, NULL);

    return MapViewOfFileEx(meta.private, access, 0, 0, (SIZE_T)meta.size, NULL);
}

void sp_UnMapFile(void* data, spf_metadata meta) {
    FlushViewOfFile(data, meta.size);
    UnmapViewOfFile(data);
    CloseHandle(meta.private);
}

typedef struct page_head {
    uint64_t size;
    char data[];
} page_head;

void* sp_PageAlloc(uint64_t size) {
    size += sizeof(page_head);
    page_head* data = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    data->size = size;
    return data->data;
}

void  sp_PageFree(void* p) {
    page_head* data = p;
    data -= 1;
    //test
    VirtualFree(data, 0, MEM_RELEASE);
}

void sp_Printf(const char* format, ...) {
}


void sp_fPrintf(sp_file f, const char* format, ...) {
}

void* sp_HeapAlloc(uint64_t size) {
    return NULL;
}

void* sp_HeapRealloc(void* p, uint64_t size) {
    return NULL;
}

void* sp_HeapCalloc(uint64_t size) {
    return NULL;
}

void sp_HeapFree(void* p) {
}

void sp_Chdir(const char* dir) {
}

void sp_Mkdir(const char* dir) {
}
#else


#define _POSIX_C_SOURCE 200809L

#include <stdarg.h>
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
        flag |= O_TRUNC;
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

void sp_ResizeFile(sp_file file, spf_metadata* meta, uint64_t newsize) {
    ftruncate(file, newsize);
    meta->size = newsize;
}

void sp_CloseFile(sp_file file) {
    close(file);
}

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

typedef struct page_head {
    uint64_t size;
    char data[];
} page_head;

void* sp_PageAlloc(uint64_t size) {
    size += sizeof(page_head);
    page_head* block = mmap(NULL, size, PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0); 
    block->size = size;
    return block->data;
}

void  sp_PageFree(void* p) {
    page_head* block = p;
    block -= 1;
    munmap(block, block->size);
}

void sp_Printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}


void sp_fPrintf(sp_file f, const char* format, ...) {
    va_list args;
    va_start(args, format);
    //file descriptor version
    vdprintf(f, format, args);
    va_end(args);
}

void* sp_HeapAlloc(uint64_t size) {
    return malloc(size);
}

void* sp_HeapRealloc(void* p, uint64_t size) {
    return realloc(p, size);
}

void* sp_HeapCalloc(uint64_t size) {
    return calloc(size, 1);
}

void sp_HeapFree(void* p) {
    free(p);
}

void sp_Chdir(const char* dir) {
    chdir(dir);
}

void sp_Mkdir(const char* dir) {
    mkdir(dir, 0664);
}

#endif
