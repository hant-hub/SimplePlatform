#ifndef SIMPLE_PLATFORM_H
#define SIMPLE_PLATFORM_H

#include <sys/types.h>

typedef int sp_file;
typedef pid_t sp_process;

//required
typedef enum sp_file_mode {
    spf_READ   = 1 << 0,
    spf_WRITE  = 1 << 1,
} sp_file_mode;

//optional
typedef enum sp_file_flags {
    spf_APPEND = 1 << 0,
    spf_TRUNC  = 1 << 0,
    spf_CREAT  = 1 << 0,
} sp_file_flags;

sp_file sp_OpenFile(const char* file, sp_file_mode mode, sp_file_flags);





#endif
