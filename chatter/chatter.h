#ifndef CHATTER_H
#define CHATTER_H


typedef struct chatter{
	int fd;
	char msg_buff[200];
	char nome[6];
} Chatter;

Chatter* conectar_sala(char*, char*);
void* enviar_mensagem(void*);
void* ouvir_sala(void*);
#endif
