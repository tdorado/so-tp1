#include "slave.h"

void call_md5(char *file_name, char *output)
{
    char command[MAX_FILE_NAME + 15];
    sprintf(command, "md5sum %256s", file_name);

    FILE *p = popen(command, "r");

    if (p == NULL){
        perror("Error: Opening file.");
        exit(EXIT_FAILURE);
    }

    sprintf(output, "%s: ", file_name);
    int i, c, aux = strlen(output);
    for (i = aux; i < (HASH_LENGTH + aux) && isxdigit(c = fgetc(p)); i++) {
        output[i] = c;
    }
    output[i] = 0;

    pclose(p);
}

void load_file(char * file_name, int pipe[2]){  
    char hash_msg[HASH_LENGTH + strlen(file_name) + 3];
    call_md5(file_name, hash_msg);
    write(pipe[1], hash_msg, strlen(hash_msg)+1);
}

void read_parent_pipe(int standards[]){
     char * msg = read_pipe(standards);
        if(msg != NULL){
            load_file(msg, standards);
            free(msg);
        }
}

void read_initial_files(char initial, int standards[]){
    for(int i=0; i<initial;i++){
        read_parent_pipe(standards);
    }
    write(STDOUT_FILENO,"-1",sizeof("-1"));
}

void read_remaining_files(char initial, int standards[]){
    //El hijo entra en ciclo hasta que el padre le indique que cierre
    while(1){
        read_parent_pipe(standards);
        write(STDOUT_FILENO,"-1",sizeof("-1"));
    }
}
