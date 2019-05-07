#ifndef _APPLICATION_H_
#define _APPLICATION_H_

//librerías de C
#include <sys/types.h>
#include <sys/unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/select.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>           /* For O_* constants */
#include <string.h>
#include <dirent.h>
#include <signal.h>

//librerías propias
#include "utilities/queue.h"
#include "utilities/utils.h"
#include "slave.h"

#define SHM_NAME "/shm"
#define NUMBER_OF_SLAVES 5
#define BLOCK 8
#define HASH_LENGTH 32
#define INITIAL_CHARGE 5

typedef struct{
  int pipe_out[2];
  int pipe_in[2];
} pipes_info;

typedef struct{
    //offset al último elemento agregado
    size_t offset, has_finished;
    sem_t semaphore;
}t_shm_info;

typedef t_shm_info * shm_info;

//crea una shared memory de tamaño SHM_MAX_SIZE y conecta la memoria con el proceso
void * create_shared_memory();

//desvincularse de la memoria compartida y limpiar todos los punteros
void clear_shared_memory(void * shm_ptr, shm_info mem_info);

/*función para inicializar el 1er bloque de la memoria compartida que es
**un puntero de tipo shm_info */
shm_info initialize_shared_memory(void * shm_ptr);

//función para guardar todo el buffer a un archivo
void save_buffer_to_file(void * shm_ptr, int n_of_files);

//guarda el hash en el archivo result.txt
void save_hash_to_file(FILE * file, char * hash);

//función para guardar un hash en la memoria compartida, utiliza semáforo
void write_hash_to_shm(void * shm_ptr, shm_info mem_info, char * hash);

//función auxiliar para calcular el tamaño total de la memoria
//no la necesitamos mas
//off_t calculate_size(int n_of_files);

//arma la queue de argumentos del proceso padre
void enqueue_args(Queue * files, int argc, char ** argv);

//envia un archivo al pipe pasado
void send_file(Queue * files, int pipe_out[]);

//abre todos los pipes desde el proceso padre
int open_pipes(pipes_info pipes[NUMBER_OF_SLAVES]);

//envia mensaje para terminar procesos hijos y cierra pipes desde el padre
void close_pipes(pid_t pids[NUMBER_OF_SLAVES], pipes_info pipes[NUMBER_OF_SLAVES]);

//realiza la creacion de hijos esclavos y deja los pipes listos
void fork_slaves(pid_t pids[NUMBER_OF_SLAVES], pipes_info pipes[NUMBER_OF_SLAVES]);

//envia los archivos iniciales a los esclavos
void send_initial_files(Queue * files, pipes_info pipes[NUMBER_OF_SLAVES]);

//función para cerrar todos los recursos del programa
void terminate_program(pid_t pids[NUMBER_OF_SLAVES], shm_info mem_info, pipes_info * pipes, Queue * files, void * shm_ptr);

//checkear si no pasan argumentos
void check_app_arguments(int argc);

//función que se encarga de procesar los archivos restantes
void send_remaining_files(FILE * file, int total_files_number, pipes_info * pipes, void * shm_ptr, shm_info mem_info, Queue * files);


#endif

