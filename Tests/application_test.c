#include "../Testing/testing_suite.h"
#include "../application.h"
#include "../utilities/utils.h"

void shared_memory_test();
void save_buffer_to_file_test();
void write_hash_to_shm_test();

int main(void){
    create_suite("Testing the Application");

    //add_test(shared_memory_test);
    add_test(save_buffer_to_file_test);
    add_test(write_hash_to_shm_test);

    run_suite();
    
    clear_suite();

    return 0;
}

//probar que puede acceder y leer desde la memoria compartida
void shared_memory_test(){
    char * result = NULL, * aux = NULL, * string = (char *) malloc(13 * sizeof(char));
    //creando memoria y escribiendola
    void * mem_ptr = create_shared_memory();
    shm_info mem_info = initialize_shared_memory(mem_ptr);
    strcpy(string, "hello world!");    
     *((char **) mem_ptr + sizeof(t_shm_info)) = (char *) string;
    int fd[2] = {0,1};
    if( pipe(fd) < 0){
        perror("Error on pipe");
        exit(EXIT_FAILURE);
    }
    //creando proceso hijo que acceda a esa memoria
    int pid = fork();
    if(pid < 0){
        perror("Error forking");
        exit(EXIT_FAILURE);
    }else if(pid == 0){ //proceso hijo
        //abrir la memoria compartida
        int shmid = shm_open(SHM_NAME, O_RDONLY, 0660);
        void * ptr = mmap(NULL, SHM_MAX_SIZE, PROT_READ, MAP_SHARED, shmid, 0);
        //decirle al padre lo que leyó
        write(fd[1], ((char **) ptr + sizeof(t_shm_info)), sizeof(char *));
        close(fd[1]);
        
        if( munmap(ptr,SHM_MAX_SIZE) == -1){
            clear_shared_memory(mem_ptr, mem_info);
            perror("Error dettaching memory");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    waitpid(pid, NULL, 0);
    read(fd[0], &result, sizeof(char *));
    //cerrar el fd y desvincular la memoria
    close(fd[0]);
    aux = *((char **) mem_ptr + sizeof(t_shm_info));
    clear_shared_memory(mem_ptr, mem_info);
    if(aux == NULL || result == NULL){
        exit(EXIT_FAILURE);
    }
    //comparar lo que escribí con lo que leyó el hijo
    assert_equals(aux,result, sizeof(char *));
}

void save_buffer_to_file_test(){
    int n_of_files = 2, ret = 0;
    char * strings = (char *) malloc((2 * HASH_NAME_SIZE) + sizeof(t_shm_info));
    char cmd_ret = 0, buff[HASH_NAME_SIZE * 2];
    memset(strings, 0, (2 * HASH_NAME_SIZE) + sizeof(t_shm_info));
    memset(buff, 0, HASH_NAME_SIZE * 2);
    strcpy(strings + sizeof(t_shm_info), "hello");
    strcpy(strings + sizeof(t_shm_info) + HASH_LENGTH, "world!");
    save_buffer_to_file(strings, n_of_files);
    //leer del archivo
    FILE * file = fopen("result.txt", "r+");
    // cppcheck-suppress invalidscanf
    fscanf(file, "%s", buff);    
    ret = strcmp(buff, strings + sizeof(t_shm_info));
    // cppcheck-suppress invalidscanf
    fscanf(file, "%s", buff + HASH_LENGTH);
    ret += strcmp(buff + HASH_NAME_SIZE, strings + HASH_NAME_SIZE + sizeof(t_shm_info));
    //liberar memoria
    free(strings);
    //borrar archivo
    call_command("rm -f result.txt", &cmd_ret);
    assert_true(!ret);
}

void write_hash_to_shm_test(){
    int  aux = 0;
    void * shm_ptr = create_shared_memory();
    shm_info mem_info = initialize_shared_memory(shm_ptr);
    char * buff = (char *) malloc(256 * sizeof(char));
    if(buff == NULL){
        perror("Malloc error");
        exit(EXIT_FAILURE);
    }
    memset(buff, 0, 256 * sizeof(char)); //limpiar todo el buffer
    call_command("md5sum ../Consigna/Sistemas_Operativos_TP1_Q1_2019.pdf", buff);
    write_hash_to_shm(shm_ptr, mem_info, buff);
    //checkear si lo que hay en memoria es lo mismo que buff
    aux = strncmp(buff, (char * )shm_ptr + sizeof(t_shm_info), HASH_NAME_SIZE);
    free(buff);
    clear_shared_memory(shm_ptr, mem_info);
    assert_true(!aux);
}
