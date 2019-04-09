#include "view.h"

void print_hashes(void * hash_start, shm_info mem_info){
	size_t offset_start = sizeof(t_shm_info);
	struct timespec ts;
	if(clock_gettime(CLOCK_REALTIME, &ts) == -1){
		perror("clock error");
		mem_disconnect(hash_start, mem_info);
		exit(EXIT_FAILURE);
	}
	ts.tv_sec += 1; //limite del semáforo de 10 segundos
	while( !mem_info->has_finished ){
		if(mem_info->offset > offset_start){
			//esperar al semáforo
			if( sem_wait(&mem_info->semaphore)){
				printf("error\n");
				printf("%s\n", strerror(errno));
				mem_disconnect(hash_start, mem_info);
				exit(EXIT_FAILURE);
			}
			printf("%s\n", (char *) hash_start + mem_info->offset - HASH_NAME_SIZE);
			mem_info->offset -= HASH_NAME_SIZE;
		}
	}
}

int open_shm(const char *name, int oflag, mode_t mode){
    int fd_shm = shm_open(name, oflag, mode); 	
	if (fd_shm == -1) {
		printf("shm_open error: ");
		printf("%s\n", strerror(errno));
		exit(EXIT_FAILURE); //exit fail?
	}
	return fd_shm;
}

void * mapping_shm(void *addr, int prot, int flags,int fd, off_t offset){
    void *ptr_shm= mmap(addr,SHM_MAX_SIZE,prot,flags,fd,offset); //retorna pointer a la nueva map area
    if(ptr_shm == (void*)-1){
        printf("Error\n");
        printf("%s\n", strerror(errno));
		exit(EXIT_FAILURE); //exit fail?
    }	
    return ptr_shm;
}

void * connect_to_shm(shm_info * mem_info){
	//agarrar el file descriptor de la shared memory
	int fd_shm = open_shm(SHM_NAME, O_RDWR, S_IRWXU); // S_IRWXU is equivalent to ‘(S_IRUSR | S_IWUSR | S_IXUSR)’.
	//mapeo la memoria con su actual size	
    void * ptr_shm = mapping_shm(NULL, PROT_READ | PROT_WRITE, MAP_SHARED,fd_shm,0);
	*mem_info = (shm_info) ptr_shm;	
	return ptr_shm;
}

void mem_disconnect(void * ptr_shm, shm_info mem_info){
	if(munmap(ptr_shm, SHM_MAX_SIZE) == -1){
		perror("munmap");
		exit(EXIT_FAILURE);
	}
    //shm_unlink(SHM_NAME);
	//close?
}

void check_pid(int argc, char ** argv){	
	int app_pid = 0;
	if(argc < 2){
		scanf("%d", &app_pid);
		if(app_pid == 0){
			printf("Application's pid must be given\n");
			exit(EXIT_FAILURE);
		}
	}else{
		//el argumento que me pasaste debe ser el pid
		app_pid = atoi(argv[1]);	
	}
	if( kill(app_pid, 0) < 0 ){
			printf("Application is not running, exiting \n");
			exit(EXIT_FAILURE);
	}
}