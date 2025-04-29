#define SB_IMPL
#include "sb.h"



int main(int argc, char* argv[]) {
    AUTO_REBUILD(argc, argv);
    sb_pick_compiler();

    //quick compile_flags.txt
    SB_FHANDLE f = sb_fopen("compile_flags.txt", "w+");
    sb_fprintf(f, "-Iinclude");
    printf("hit\n");

    //create build dir
    sb_mkdir("build");

    sb_cmd* c = &(sb_cmd){0};
    sb_cmd_push(c, sb_compiler());
    sb_cmd_push(c, "src/platform.c");
    sb_cmd_push(c, "test/fileio.c");
    sb_cmd_push(c, sb_cmd_flag("Iinclude"));
    sb_cmd_push(c, sb_cmd_flag("g"));
    sb_cmd_push(c, sb_cmd_flag("o"), "build/test");
    sb_cmd_sync_and_reset(c);
    
    sb_cmd_push(c, "./build/test");
    sb_cmd_sync_and_reset(c);


    sb_cmd_free(c);
    return 0;
}
