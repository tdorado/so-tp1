#include "../utils.h"

//utils.c , comandos auxiliares que puede usar cualquier programa

void call_command(char * command, char * buffer){
    if(command == NULL || buffer == NULL){
        return;
    }
    FILE * fd;
    fd = popen((char *) command, "r");
    int i = 0;
    int c = 0;
    while ( (c=getc(fd)) != '\n' && c!=EOF ){
        buffer[i] = c;
        i++;
    }
    buffer[i] = '\n';
    buffer[++i] = '\0';
    pclose(fd);
    return;
}

char * read_pipe(int pipe[2]){
    int i = 0;
    char * msg;
    char c = 1;

    msg = malloc(BLOCK);

    while(c != 0){
        int r = read(pipe[0], &c, 1);
        if(r > 0){
            if(i%BLOCK==0){
                char * aux = msg;
                msg = realloc(aux, i + BLOCK);
                if(msg == NULL){
                    perror("Error of memory.");
                    exit(EXIT_FAILURE);
                }
            }
            msg[i++] = c;
        }
        else{
            perror("Error reading pipe.");
            exit(EXIT_FAILURE);
        }
    }

    if(i%BLOCK==0){
        char * aux = msg;
        msg = realloc(aux, i + 1);
        if(msg == NULL){
            perror("Error of memory.");
            exit(EXIT_FAILURE);
        }
    }
    msg[i] = 0;
    return msg;
}
