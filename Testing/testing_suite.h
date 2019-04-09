#ifndef _TESTING_SUITE_H_
#define _TESTING_SUITE_H_

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <execinfo.h>
#include "tasteful.h"

#define ARRAY_STEP 5

enum STATE {SUCCESS = 0, FAILURE}; //ver si hay que crear un 3er estado default

typedef struct{
    //punteros a puntero a función
    void  (** fun_ptrs) (void);
    //cantidad de tests que contiene
    int n_of_tests;
    //el estado final una vez corridos los tests (por defecto SUCCESS)
    enum STATE suite_state; 
    //path del suite
    char * suite_name;
    /*índice del array (podría usarlo a futuro para que sea más cómodo
    **agregar funciones)*/
    int fun_index;
}t_test_suite;

typedef t_test_suite * test_suite;

//los asserts posibles que se pueden hacer

//verifica si el elemento a es igual al elemento b indicando el tamaño del dato
void assert_equals(const void * a, const void * b, size_t size_of_data);

//verifica si el elemento a es distinto al elemento b indicando el tamaño del dato
void assert_not_equals(const void * a, const void * b, size_t size_of_data);

//verifica si el resultado de la operación que se le evalúe es verdadera
void assert_true(int i);

//verifica si el resultado de la operación que se le evalúe es falsa
void assert_false(int i);

//correr todas los tests del suite
void run_suite();

//función para inicializar la suite
void create_suite(char * suite_name);

//función para agregar un test al suite
void add_test(void (* fun) ());

//función que se encarga de hacer todos los free
void clear_suite();

//imprimir las últimadas llamadas al stack
void print_trace();

//verificar el estado del proceso hijo
void check_child_status(int status);

//puntero global para que sea más cómodo operar con la estructura
test_suite suite;

#endif
