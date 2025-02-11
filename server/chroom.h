#ifndef CHROOM_H
#define CHROOM_H

#include <poll.h>
#include <stddef.h>

typedef struct chroom{
	int fd;
	int capacidade;
	int atual;
	char* nomes;
	struct pollfd* conexoes;
	char* msg_buff;
	size_t msg_size;
	int max_size;
} Chroom;

extern int exit_flag;

Chroom* abrir_sala(char*);
void* aceitar_chatter(void*);
int ouvir_mensagem(Chroom*, int);
void* ouvir_chatters(void*);
void ecoar_mensagem(Chroom*, int, int);
void fechar_sala(Chroom*);
#endif
