#include "../Testing/testing_suite.h"
#include "../application.h"
#include "../utilities/utils.h"
#include "../view.h"

void app_vision_integration_test();
void write_and_read_continuous_test();
void ejemplo_profe();
void multiple_write_with_sleep();

int main(void){
    create_suite("Testing the Application & Vision Integration");

    add_test(app_vision_integration_test);
    //add_test(ejemplo_profe);
    add_test(write_and_read_continuous_test);
    add_test(multiple_write_with_sleep);

    run_suite();
    
    clear_suite();

    return 0;
}


void app_vision_integration_test(){
    //como si estuvieramos arrancando application
    int cpid = 0, mypid = getpid(), child_status = 0, fd[2] = {0,1}, stdout_saved = dup(STDOUT_FILENO);
    void * shm_ptr = create_shared_memory();
    char buffer[HASH_NAME_SIZE], * const str = (char * const) malloc(5 * sizeof(char));
    memset(buffer, 0, HASH_NAME_SIZE);
    shm_info mem_info = initialize_shared_memory(shm_ptr);
    if(pipe(fd) < 0){
      perror("pipe");
      exit(EXIT_FAILURE);
    }
    //agarramos un hash llamando a md5sum
    char * buff = (char *) malloc(256 * sizeof(char));
    if(buff == NULL){
        perror("Malloc error");
        exit(EXIT_FAILURE);
    }
    memset(buff, 0, 256 * sizeof(char)); //limpiar todo el buffer
    call_command("md5sum ../Sistemas_Operativos_TP1_Q1_2019.pdf", buff);
    write_hash_to_shm(shm_ptr, mem_info, buff);
    mem_info->has_finished = 1; //se terminaron los archivos
    //cerrando stdout
    close(STDOUT_FILENO);
    dup(fd[1]);
    close(fd[1]);

    //imprimir hash desde la view
    cpid = fork();
    if(cpid < 0){
        perror("Fork error");
        exit(EXIT_FAILURE);
    }else if(cpid == 0){ //hijo
        sprintf(str, "%d", mypid);
        char * argv[] = {"../view.so", str, NULL};
        if( execv("../view.so", argv) == -1){
            free(str);
            perror("Excec error");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    waitpid(cpid, &child_status, 0);
    //reestablecer stdout
    fflush(stdout);
    dup2(stdout_saved, STDOUT_FILENO);
    close(stdout_saved);
    //leer el printf
    read(fd[0], buffer, HASH_NAME_SIZE);
    close(fd[0]);
    clear_shared_memory(shm_ptr, mem_info);
    //check_child_status(child_status);
    free(str);
    free(buff);
    assert_true(!strncmp(buffer,
                        "dbbc672b0dec675712e78f98cfe88c25  ../Sistemas_Operativos_TP1_Q1_2019.pdf\n",
                        strlen("dbbc672b0dec675712e78f98cfe88c25  ../Sistemas_Operativos_TP1_Q1_2019.pdf\n")));
}

void ejemplo_profe(){
    int fd = shm_open("/SHM_NAME", O_CREAT | O_RDWR, S_IRWXU), cpid = 0;
  ftruncate(fd, 100 * sizeof(char));
  void *shm_ptr = mmap(NULL, 100 * sizeof(char), PROT_WRITE, MAP_SHARED, fd, 0);
  printf("parent ptr %p \n", shm_ptr);
  cpid = fork();

  if (cpid < 0) {
    perror("fork error");
    exit(EXIT_FAILURE);
  } else if (cpid == 0) {  // proceso hijo
    int fd2 = shm_open("/SHM_NAME", O_RDONLY, S_IRWXU);
    void *shm_ptr2 = mmap(NULL, 100 * sizeof(char), PROT_READ, MAP_SHARED, fd2, 0);
    printf("child ptr %p \n", shm_ptr2);
    // cada 1 segundo el child lee el valor del primer byte de la memoria
    char i;
    char *p = (char *) shm_ptr2;
    for (i = 0; i < 3; i++){
      printf("Child read: %d\n", *p);
      sleep(1);
    }
    munmap(shm_ptr2, 100 * sizeof(char *));
    // shm_unlink("/SHM_NAME");
    exit(EXIT_SUCCESS);
  } else{
    //cada 1 segundo parent escribe un nuevo valor al inicio de la memoria
    char i;
    char *p = (char *) shm_ptr;
    for (i = 0; i < 3; i++){
      *p = i;
      sleep(1);
    }
  }
  waitpid(cpid, NULL, 0);
  munmap(shm_ptr, 100 * sizeof(char *));
  shm_unlink("/SHM_NAME");
}

//escribir un archivo, esperar y escribir otro mientras vision lee
void write_and_read_continuous_test(){
  int  cpid = 0, mypid = getpid(), fd[2] = {0,1}, stdout_saved = dup(STDOUT_FILENO), aux = 0;
  void * shm_ptr = create_shared_memory();
  char buffer[2][HASH_NAME_SIZE];
  memset(buffer[0], 0, HASH_NAME_SIZE);
  memset(buffer[1], 0, HASH_NAME_SIZE);
  shm_info mem_info = initialize_shared_memory(shm_ptr);

  if(pipe(fd) < 0){
      perror("pipe");
      exit(EXIT_FAILURE);
  }

  char * buff = (char *) malloc(256 * sizeof(char)), * const str = (char * const) malloc(5 * sizeof(char));
  if(buff == NULL){
      perror("Malloc error");
      exit(EXIT_FAILURE);
  }
  memset(buff, 0, 256 * sizeof(char)); //limpiar todo el buffer
  call_command("md5sum ../Sistemas_Operativos_TP1_Q1_2019.pdf", buff);
  write_hash_to_shm(shm_ptr, mem_info, buff);
  //cerrando stdout
  close(STDOUT_FILENO);
  dup(fd[1]);
  close(fd[1]);
  
  //ejecutar view.c
  cpid = fork();
  if(cpid < 0){
    perror("Fork error");
    exit(EXIT_FAILURE);
  }else if(cpid == 0){ //hijo
    sprintf(str, "%d", mypid);
    char * argv[] = {"../view.so", str, NULL};
    if( execv("../view.so", argv) == -1){
      perror("Excec error");
      exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
  }
  //reestablecer stdout
  fflush(stdout);
  dup2(stdout_saved, STDOUT_FILENO);
  close(stdout_saved);
  //limpiar y volver a escribir al buffer
  memset(buff, 0, 256 * sizeof(char));
  call_command("md5sum ../README.md", buff);
  write_hash_to_shm(shm_ptr, mem_info, buff);
  mem_info->has_finished = 1;
  //leer el printf
  read(fd[0], buffer[0], 73 * sizeof(char));
  read(fd[0], buffer[1], 48 * sizeof(char));
  close(fd[0]);
  //comparar
  aux = strcmp("dbbc672b0dec675712e78f98cfe88c25  ../Sistemas_Operativos_TP1_Q1_2019.pdf\n", buffer[0]);
  aux += strcmp(buffer[1], " \nb1c856e726f0368927badab296266c8d  ../README.md");
  //limpiar y liberar memorias
  clear_shared_memory(shm_ptr, mem_info);
  free(str);
  free(buff);
  assert_true(!aux);
}

//Test que prueba si vision se va actualizando si el padre escribe después
void multiple_write_with_sleep(){
  int  cpid = 0, mypid = getpid(), fd[2] = {0,1}, stdout_saved = dup(STDOUT_FILENO), aux = 0;
  void * shm_ptr = create_shared_memory();
  char buffer[HASH_NAME_SIZE], * commands[3], * const str = (char * const) malloc(5 * sizeof(char));  
  char output[3][HASH_NAME_SIZE];
  memset(output[0], 0, HASH_NAME_SIZE);
  memset(output[1], 0, HASH_NAME_SIZE);
  memset(output[2], 0, HASH_NAME_SIZE);
  commands[0] = "md5sum ../Sistemas_Operativos_TP1_Q1_2019.pdf";
  commands[1] = "md5sum ../README.md";
  commands[2] = "md5sum ./hashing_file.txt";;
  memset(buffer, 0, HASH_NAME_SIZE);
  shm_info mem_info = initialize_shared_memory(shm_ptr);
  char * buff = (char *) malloc(256 * sizeof(char));
  if(buff == NULL){
      perror("Malloc error");
      exit(EXIT_FAILURE);
  }
  memset(buff, 0, 256 * sizeof(char)); //limpiar todo el buffer
  call_command(commands[0], buff);
  write_hash_to_shm(shm_ptr, mem_info, buff);
  //creando pipe
  if(pipe(fd) < 0){
      perror("pipe");
      exit(EXIT_FAILURE);
  }
  //cerrando stdout
  close(STDOUT_FILENO);
  dup(fd[1]);
  close(fd[1]);

  //imprimir hash desde la view
  cpid = fork();
  if(cpid < 0){
    perror("Fork error");
    exit(EXIT_FAILURE);
  }else if(cpid == 0){ //hijo
    sprintf(str, "%d", mypid);
    char * argv[] = {"../view.so", str, NULL};
    if( execv("../view.so", argv) == -1){
      perror("Excec error");
      exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
  }

  //reestablecer stdout
  fflush(stdout);
  dup2(stdout_saved, STDOUT_FILENO);
  close(stdout_saved);

  for(int i = 1; i < 3; i++){
    sleep(1);
    memset(buff, 0, 256 * sizeof(char));
    call_command(commands[i], buff);
    write_hash_to_shm(shm_ptr, mem_info, buff);
  }
  mem_info->has_finished = 1;
  waitpid(cpid, NULL, 0);
  read(fd[0], output[0], 73 * sizeof(char));
  read(fd[0], output[1], 48 * sizeof(char));
  read(fd[0], output[2], 55 * sizeof(char));
  aux =  strcmp(output[0],  "dbbc672b0dec675712e78f98cfe88c25  ../Sistemas_Operativos_TP1_Q1_2019.pdf\n");  
  aux += strcmp(output[1], " \nb1c856e726f0368927badab296266c8d  ../README.md");  
  aux += strcmp(output[2], "\n \n807a23aca22344e4426f1f73eb2c0109  ./hashing_file.txt");
  //limpiar la memoria
  clear_shared_memory(shm_ptr, mem_info);
  free(str);
  free(buff);
  assert_true(!aux);
}