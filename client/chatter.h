#ifndef CHATTER_H
#define CHATTER_H

#include <stddef.h>

typedef struct chatter{
	int fd;
	char* buff_in;
	size_t in_size;
	char* buff_out;
	size_t out_size;
} Chatter;

extern int exit_flag;

Chatter* criar_chatter();
void ler_input(Chatter*);
Chatter* conectar_sala(char*, char*);
void* enviar_mensagem(void*);
void* ouvir_sala(void*);
void apagar_chatter(Chatter*);
#endif
