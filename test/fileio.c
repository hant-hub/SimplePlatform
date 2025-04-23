#include <platform.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

int main() {

    spf_metadata meta;
    sp_file f = sp_OpenFile("sb_build.c", spf_READ, 0, &meta);
    char* data = sp_MemMapFile(f, meta, spm_READ | spm_PRIVATE);
    sp_Printf("%.*s\n", (int)meta.size, data);
    sp_UnMapFile(data, meta);
    sp_CloseFile(f);

    f = sp_OpenFile("test_output.txt", spf_WRITE | spf_READ, spf_CREAT | spf_TRUNC, &meta);
    data = sp_MemMapFile(f, meta, spm_WRITE | spm_READ | spm_SHARED);
    sp_ResizeFile(f, &meta, 64);
    strcpy(data, "some test text\nsome more text\nand even more text\n");
    sp_UnMapFile(data, meta);
    sp_CloseFile(f);

    data = sp_PageAlloc(100);
    for (int i = 0; i < 100; i++) {
        data[i] = 'i';
    }
    sp_Printf("%.*s\n", 100, data);
    sp_PageFree(data);

    return 0;
}
