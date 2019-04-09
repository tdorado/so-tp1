#include "slave.h"

int main(void){
    int standards[2] = {STDIN_FILENO, STDOUT_FILENO};
    char initial;
    read(STDIN_FILENO, &initial, sizeof(char));
    read_initial_files(initial, standards); 
    read_remaining_files(initial, standards);
}