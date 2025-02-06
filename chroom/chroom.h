#ifndef CHROOM_H
#define CHROOM_H

#include <poll.h>

typedef struct chroom{
	int fd;
	struct pollfd conexoes[10];
	char msg_buff[204];
} Chroom;

Chroom* abrir_sala(char*);
void* aceitar_chatter(void*);
void* ouvir_chatters(void*);
void fechar_sala(Chroom*);
#endif
