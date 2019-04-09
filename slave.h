#ifndef _SLAVE_H_
#define _SLAVE_H_

//librerías de C
#include <stdio.h>
#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

//librerías propias
#include "utilities/utils.h"

//función para calcular el hash md5 en base a un archivo retorna 'nombre archivo: hash'
void call_md5(char * file_name, char * output);

//recibe el mensaje con el archivo y la pipe de entrada para enviar al padre
void load_file(char * file_name, int pipe[2]);

//función para leer del pipe el archivo y procesarlo
void read_parent_pipe(int standards[]);

//leer la primera carga de archivos
void read_initial_files(char initial, int standards[]);

//leer los demás archivos
void read_remaining_files(char initial, int standards[]);

#endif
