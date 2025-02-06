#include "chatter.h"
#include "../conexao.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


Chatter* criar_chatter(char* nome){
	Chatter* chatter = malloc(sizeof(*chatter));
	strncpy(chatter->nome, nome, sizeof(chatter->nome));
	chatter->nome[4] = '\0';

	return chatter;
}

Chatter* conectar_sala(char* ip, char* port){
	char nome[6];
	printf("Digite um nome de ate 4 letras para te identificar: ");
	fgets(nome, sizeof(nome), stdin);
	Chatter* chatter = criar_chatter(nome);
	chatter->fd = encontrar_conexao(ip, port);

	if(chatter->fd == -1){
		return NULL;
	}

	return chatter;
}


void* enviar_mensagem(void* ptr){
	Chatter* chatter = (Chatter*)ptr;
	char msg[210];
	while(1){
		//printf("%s: ", chatter->nome);
		memset(msg, 0, sizeof(msg));
		strncpy(msg, chatter->nome, 4);
		strcat(msg, ": ");
		fgets(chatter->msg_buff, sizeof(chatter->msg_buff), stdin);
		strncat(msg, chatter->msg_buff, sizeof(chatter->msg_buff));
		send(chatter->fd, msg, sizeof(msg), 0);		
	}
}


void* ouvir_sala(void* ptr){
	Chatter* chatter = (Chatter*)ptr;
	while(1){
		while(recv(chatter->fd, (char*)chatter->msg_buff, sizeof(chatter->msg_buff), 0)){
			printf("%s", chatter->msg_buff);	
			memset(chatter->msg_buff, 0, sizeof(chatter->msg_buff));
		}
	}
}
