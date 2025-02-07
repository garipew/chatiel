#ifndef CHROOM_H
#define CHROOM_H

#include <poll.h>

typedef struct chroom{
	int fd;
	int capacidade;
	int atual;
	char* nomes;
	struct pollfd* conexoes;
	char msg_buff[204];
} Chroom;

extern int exit_flag;

Chroom* abrir_sala(char*);
void* aceitar_chatter(void*);
void* ouvir_chatters(void*);
void fechar_sala(Chroom*);
#endif
