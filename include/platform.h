#ifndef SIMPLE_PLATFORM_H
#define SIMPLE_PLATFORM_H



#include <stdint.h>
#include <sys/types.h>

//platform dependent------------
typedef int sp_file;
typedef pid_t sp_process;
//------------------------------

typedef uint32_t Bool32;
#define TRUE 1
#define FALSE 0

//required
typedef enum sp_file_mode {
    spf_READ   = 1 << 0,
    spf_WRITE  = 1 << 1,
} sp_file_mode;

//optional
typedef enum sp_file_flags {
    spf_APPEND = 1 << 0,
    spf_TRUNC  = 1 << 1,
    spf_CREAT  = 1 << 2,
} sp_file_flags;

typedef struct spf_metadata {
    uint64_t size;
} spf_metadata;

sp_file sp_OpenFile(const char* file, sp_file_mode mode, sp_file_flags flags, spf_metadata* meta);
void sp_CloseFile(sp_file file);

typedef enum sp_map_flags {
    spm_READ    = 1 << 0,
    spm_WRITE   = 1 << 1,
    spm_PRIVATE = 1 << 2,
    spm_SHARED  = 1 << 3,
} sp_map_flags;

//NOTE(ELI): To create and write a file of a specific length,
//just modify the metadata to the correct length
void* sp_MemMapFile(sp_file f, spf_metadata meta, sp_map_flags flags);
void sp_UnMapFile(void* data, spf_metadata meta);



#endif
