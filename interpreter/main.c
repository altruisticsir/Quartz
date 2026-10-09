#ifdef _WIN32
    #define SANDBOXED
    // #define VERBOSE
    #include <stdio.h>
#endif

#include "lib/exec.h"

int main(void) {
    exec_init();
    
    #ifdef SANDBOXED
        /* Sandboxed; should read from a bytecode
        file in the directory */
        const char *path = "bin/qz.bin";
        FILE *file = fopen(path, "rb");
        
        if (file == NULL) {
            /* Attempt the open the file was unsuccessful */
            printf("Could not open file \"%s\"\n", path);
            return 1;
        }

        int c;

        while ((c = fgetc(file)) != EOF) {
            exec_write((uint8_t)c);
        }

        fclose(file);
    #endif

    for (;;) exec_step();

    return 0;
}