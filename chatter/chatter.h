#ifndef CHATTER_H
#define CHATTER_H


typedef struct chatter{
	int fd;
	char msg_buff[200];
} Chatter;

extern int exit_flag;

Chatter* conectar_sala(char*, char*);
void* enviar_mensagem(void*);
void* ouvir_sala(void*);
void apagar_chatter(Chatter*);
#endif
