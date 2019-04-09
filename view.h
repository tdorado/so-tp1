#ifndef _VIEW_H_
#define _VIEW_H_

//librerías de C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h> 
#include <sys/stat.h> /* For mode constants */ 
#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>

//librerías propias
#include "application.h"
#include "utilities/utils.h"


//opens or creates an existing shared memory object.
//returns a nonnegative file descriptor
int open_shm(const char *name, int oflag, mode_t mode);

//creates a new mapping in the virtual address space of the calling process
//returns a pointer to the mapped area
void * mapping_shm(void *addr, int prot, int flags,int fd, off_t offset);

//creates a new semaphore or opens an existin semaphore
//returns the address of the new or existing semaphore
sem_t *open_sem(const char *name, int oflag, mode_t mode, unsigned int value);

void print_hashes(void * hash_start, shm_info mem_info);

//función para conectar este proceso con la memoria compartida
void * connect_to_shm(shm_info * mem_info);

//función para desconectarse a la memoria compartida
void mem_disconnect(void * ptr_shm, shm_info mem_info);

//función para verificar la cantidad de argumentos de vision
void check_pid(int argc, char ** argv);

#endif
