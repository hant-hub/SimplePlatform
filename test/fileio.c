#include <linux/falloc.h>
#define _GNU_SOURCE

#include <platform.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

int main() {

    spf_metadata meta;
    sp_file f = sp_OpenFile("sb_build.c", spf_READ, 0, &meta);
    char* data = sp_MemMapFile(f, meta, spm_READ | spm_PRIVATE);
    printf("%.*s\n", (int)meta.size, data);
    sp_UnMapFile(data, meta);

    sp_CloseFile(f);

    f = sp_OpenFile("test_output.txt", spf_WRITE | spf_READ, spf_CREAT | spf_TRUNC, &meta);
    data = sp_MemMapFile(f, meta, spm_WRITE | spm_READ | spm_SHARED);
    perror("map");
    strcpy(data, "some test text\nsome more text\nand even more text\n");
    sp_CloseFile(f);


    return 0;
}
