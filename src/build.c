#define NOB_IMPLEMENTATION
#include "../external/nob.h"

int main(int argc, char** argv){
    GO_REBUILD_URSELF(argc, argv);
    Cmd cmd = {0};

    cmd_append(&cmd,"g++","src/main.cpp","-std=c++20");
    cmd_append(&cmd,"-lraylib","-lm","-lpthread","-ldl","-lrt","-lX11");
    cmd_append(&cmd,"-o","build/layd");

    if(!cmd_run(&cmd)){
        fprintf(stderr,"Build failed\n");
        return 1;
    }

    cmd_append(&cmd,"./build/layd");
    if(!cmd_run(&cmd)){
        fprintf(stderr,"Execution failed\n");
        return 1;
    }
    return 0;
}