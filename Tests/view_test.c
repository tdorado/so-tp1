#include "../Testing/testing_suite.h"
#include "../application.h"
#include "../view.h"

void get_shm_info_test();

int main(void){
    create_suite("Testing the View");

    add_test(get_shm_info_test);

    run_suite();
    
    clear_suite();

    return 0;
}

void get_shm_info_test(){
    int  cpid = 0, fds[2] = {0,1}, ret = 0;
    void * shm_ptr = create_shared_memory();
    shm_info mem_info = initialize_shared_memory(shm_ptr);
    if(pipe(fds) < 0){
        perror("pipe error");
        exit(EXIT_FAILURE);
    }
    cpid = fork();
    if(cpid < 0){
        perror("fork error");
        exit(EXIT_FAILURE);
    }else if(cpid == 0){
        shm_info target = NULL;
        void * shm_ptr = connect_to_shm(&target);
        ret = memcmp(mem_info, target, sizeof(t_shm_info));
        write(fds[1], &ret, sizeof(shm_info));
        close(fds[1]);
        //shm_unlink(SHM_NAME);
        //falta el close?
        munmap(shm_ptr, sizeof(t_shm_info));
        exit(EXIT_SUCCESS);
    }
    waitpid(cpid, NULL, 0);
    read(fds[0], &ret, sizeof(shm_info));    
    clear_shared_memory(shm_ptr, mem_info);
    assert_true(!ret);
}