#include "view.h"

int main(int argc, char ** argv){
	check_pid(argc, argv);
	shm_info mem_info = NULL;
	void * ptr_shm = connect_to_shm(&mem_info);
	print_hashes(ptr_shm, mem_info);
	mem_disconnect(ptr_shm, mem_info);
	return 0;
}


