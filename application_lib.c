#include "application.h"

void * create_shared_memory(){
    void * shm_ptr = NULL;
    //crear la memoria compartida
    int shmid = 0;    
    shmid = shm_open(SHM_NAME, O_RDWR | O_CREAT, S_IRWXU);
    if(shmid < 0){
        perror("smh_open");
        exit(EXIT_FAILURE);
    }
    //definir el tamaño de la memoria (más el bloque inicial)
    if( ftruncate(shmid, SHM_MAX_SIZE) == -1 ){ //que el tamaño sea múltiplo de una pag?
        perror("can't truncate");
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    } 
    
    if( (shm_ptr = (void *) mmap(NULL, SHM_MAX_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, shmid, 0)) == MAP_FAILED){
        //no se puede conectar con la memoria
        perror("can't attach memory");
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }
    return shm_ptr;
}

shm_info initialize_shared_memory(void * shm_ptr){
    t_shm_info shm_info;
    //apuntar a donde empiezan los hashes
    shm_info.offset = sizeof(t_shm_info); 
    shm_info.has_finished = 0;
    //inicializando el semáforo con valor 0
    if (sem_init(&shm_info.semaphore, 1, 0 ) < 0 ){
        perror("Error initializing semaphore");
        unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }
    memcpy(shm_ptr, &shm_info, sizeof(t_shm_info));    
    return shm_ptr;
}

void clear_shared_memory(void * shm_ptr, shm_info mem_info){  
    sem_destroy(&mem_info->semaphore);
    munmap(shm_ptr,SHM_MAX_SIZE);
    shm_unlink(SHM_NAME);
}

void save_buffer_to_file(void * shm_ptr, int n_of_files){
    FILE * file = fopen("result.txt", "w+a+");
    int i;
    for(i = 0; i < n_of_files;i++){
        fprintf(file, "%s \n", ((char *)shm_ptr + sizeof(t_shm_info) + i * HASH_NAME_SIZE));
    }
    fclose(file);
}

void save_hash_to_file(FILE * file, char * hash){
    fprintf(file, "%s\n", hash);
}

void write_hash_to_shm(void * shm_ptr, shm_info mem_info, char * hash){   
    //escribir
    strcpy((char *) shm_ptr + mem_info->offset, hash);
    //desplazarse
    mem_info->offset += HASH_NAME_SIZE;
    //avisar que hay algo escrito
    if( sem_post(&mem_info->semaphore) < 0){
        perror("Error in wait");
        clear_shared_memory(shm_ptr, mem_info);
        exit(EXIT_FAILURE);
    } 
}

// funcion interna recursiva para encolar archivos y directorios 
void enqueue_rec(Queue * files, char * file_name){
    struct stat path_stat;
    stat(file_name, &path_stat);
    // usamos la macro para ver si es una file
    if(S_ISREG(path_stat.st_mode)){
        // si es una file es facil, la encolamos
        enqueue(files ,&file_name);
    }
    // usamos otra macro para ver si es un directorio
    else if(S_ISDIR(path_stat.st_mode)){
        // si es un dir vemos todas las files adentro
        DIR * dir;
        struct dirent * ent;
        dir = opendir(file_name);
        while((ent = readdir(dir)) != NULL){
            if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..")){
                char * new_file_name = malloc(strlen(ent->d_name) + strlen(file_name) + 2);
                // agrego al nuevo path
                if(file_name[strlen(file_name)] == '/'){
                    sprintf(new_file_name, "%s%s", file_name, ent->d_name);
                }
                else{
                    sprintf(new_file_name, "%s/%s", file_name, ent->d_name);
                }
                // llamamos con el nuevo path
                enqueue_rec(files, new_file_name);
            }
        }
        free(file_name);
        closedir(dir);
    }
}

void enqueue_args(Queue * files, int argc, char ** argv){
    int i;
    for(i=1; i<argc; i++){
        char * file_name = malloc(strlen(argv[i]) + 1);
        strcpy(file_name,argv[i]);
        enqueue_rec(files, file_name);
    }
}

void send_file(Queue * files, int pipe_out[]){
    char * file_name;
    dequeue(files, &file_name);
    write(pipe_out[1], file_name, strlen(file_name)+1);
    free(file_name);
}

int open_pipes(pipes_info pipes[NUMBER_OF_SLAVES]){
    // abrimos todos los pipes
    int i;
    for(i=0; i<NUMBER_OF_SLAVES; i++){
        if(pipe(pipes[i].pipe_out)==-1 || pipe(pipes[i].pipe_in)==-1){
            return -1;
        }
    }
    return 0;
}

void close_pipes(pid_t pids[NUMBER_OF_SLAVES], pipes_info pipes[NUMBER_OF_SLAVES]){
    int i;
    for(i=0; i<NUMBER_OF_SLAVES;i++){
        //Cerramos el final de lectura del pipe de entrada
        close(pipes[i].pipe_in[0]);
        //Cerramos el final de escritura del pipe de salida
        close(pipes[i].pipe_out[1]);
        //Matamos proceso hijo
        kill(pids[i], SIGKILL);
    }
}

void fork_slaves(pid_t pids[NUMBER_OF_SLAVES], pipes_info pipes[NUMBER_OF_SLAVES]){
    int i;
    // padre crea procesos esclavos y pipes
    for(i=0; i<NUMBER_OF_SLAVES; i++){
        pids[i]=fork();
        if(pids[i]<0){
            perror("Error: Fork failed.");
            exit(EXIT_FAILURE);
        }
        else if(pids[i]==0){ // proceso hijo/esclavo
            //Cerramos el final de escritura del pipe de salida
            close(pipes[i].pipe_out[1]);
            //Cerramos el final de lectura del pipe de entrada
            close(pipes[i].pipe_in[0]);
            // cerramos stdin
            close(STDIN_FILENO);
            // redireccionamos stdin del slave al final de lectura
            // del pipe de salida
            dup(pipes[i].pipe_out[0]);
            //cerramos stdout
            close(STDOUT_FILENO);
            // redireccionamos stdout del slave al final de escritura
            // del pipe de entrada
            dup(pipes[i].pipe_in[1]);
            char ** no_args = {'\0'};
            execv("slave.so", no_args);
        }
        else{ // proceso padre
            //Cerramos el final de lectura del pipe de salida
            close(pipes[i].pipe_out[0]);
            //Cerramos el final de escritura del pipe de entrada
            close(pipes[i].pipe_in[1]);
        }
    }
}

void send_initial_files(Queue * files, pipes_info pipes[NUMBER_OF_SLAVES]){
    int i;
    if(INITIAL_CHARGE*NUMBER_OF_SLAVES <= getQueueSize(files)){
        int j;
        for(i=0; i<NUMBER_OF_SLAVES; i++){
            char initial = INITIAL_CHARGE;
            write(pipes[i].pipe_out[1],&initial,sizeof(initial));
            for(j=0; j<INITIAL_CHARGE;j++){
                send_file(files,pipes[i].pipe_out);
            }
        }
    }
    else{
        char initial = 1;
        for(i=0; i<NUMBER_OF_SLAVES && getQueueSize(files) > 0; i++){
            write(pipes[i].pipe_out[1],&initial,sizeof(initial));
            send_file(files,pipes[i].pipe_out);
        }
    }   
}

void terminate_program(pid_t pids[NUMBER_OF_SLAVES], shm_info mem_info, pipes_info * pipes, Queue * files, void * shm_ptr){
    mem_info->has_finished = 1;

    close_pipes(pids, pipes);

    freeQueue(files);

    //desvincularse a la memoria y liberarla
    clear_shared_memory(shm_ptr, mem_info);
}

void check_app_arguments(int argc){
    // si no tenemos argumentos no hay nada que hacer
    if(argc < 2){
        printf("No files in arguments \n");
        exit(EXIT_FAILURE);
    }
}

void send_remaining_files(FILE * file, int total_files_number, struct timeval tv, pipes_info * pipes, fd_set read_set, void * shm_ptr, shm_info mem_info, Queue * files){
    char * hash = NULL;
    // cppcheck-suppress variableScope
    while(total_files_number>0){
        tv.tv_sec=10;
        tv.tv_usec=0;
        
        for(int i=0; i<NUMBER_OF_SLAVES;i++){
            FD_SET(pipes[i].pipe_in[0],&read_set);
        }
        //el primer argumento de select debe ser el pipe mas grande de todos + 1, aclarado en libreria de select
        int maxfd = pipes[0].pipe_in[1];
        for(int i=1; i<NUMBER_OF_SLAVES;i++){
            if(maxfd < pipes[i].pipe_in[1]){
                maxfd = pipes[i].pipe_in[1];
            }
        }
        if(select(maxfd+1, &read_set, NULL, NULL, &tv) < 0){
            perror("Error: Select failed.\n");
            exit(EXIT_FAILURE);
        }
        for(int i=0; i<NUMBER_OF_SLAVES && total_files_number>0; i++){
            if(FD_ISSET(pipes[i].pipe_in[0],&read_set)){
                hash = read_pipe(pipes[i].pipe_in);
                if(hash != NULL && strcmp(hash,"-1")!=0){
                    total_files_number--;
                    write_hash_to_shm(shm_ptr, mem_info, hash);
                    save_hash_to_file(file, hash);
                }
                else if(getQueueSize(files)>0 && hash!=NULL && strcmp(hash,"-1")==0){
                    send_file(files,pipes[i].pipe_out);
                }
                free(hash);
            }
        }
    }
}
