#include "application.h"

int main(int argc, char ** argv){

    check_app_arguments(argc);

    //imprimir el pid para vision
    printf("%d\n", getpid());

    // creamos e inicializamos la shm
    void * shm_ptr = create_shared_memory();
    shm_info mem_info = initialize_shared_memory(shm_ptr);
    fd_set read_set;
    struct timeval tv = {10, 0};
    pipes_info pipes[NUMBER_OF_SLAVES];
    Queue * files = newQueue();
    FILE * file = fopen("result.txt", "w+a+");
    int total_files_number;
    pid_t pids[NUMBER_OF_SLAVES];

    // inicializamos para select
    FD_ZERO(&read_set);

    //inicializamos la queue de files
    queueInit(files, sizeof(char*));
    enqueue_args(files, argc, argv);
    total_files_number = getQueueSize(files);

    //dormir para que vision se pueda conectar
    sleep(1);
    fflush(stdout);

    if(open_pipes(pipes) == -1){
        perror("Error: Pipe failed.");
        exit(EXIT_FAILURE);
    }

    fork_slaves(pids, pipes);
    send_initial_files(files, pipes);
    send_remaining_files(file, total_files_number, tv, pipes, read_set, shm_ptr, mem_info, files);
    terminate_program(pids, mem_info, pipes, files, shm_ptr);
    fclose(file);

    exit(EXIT_SUCCESS);
}
