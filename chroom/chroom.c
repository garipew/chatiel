#include "chroom.h"
#include "../conexao.h"
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int exit_flag = 0;


Chroom* abrir_sala(char* port){
	Chroom* sala = malloc(sizeof(*sala));

	sala->fd = encontrar_conexao(NULL, port);
	sala->capacidade = 5;
	sala->atual = 0;
	sala->conexoes = malloc(sizeof(*(sala->conexoes)) * sala->capacidade);
	sala->nomes = malloc(sizeof(*(sala->nomes)) * sala->capacidade * 4);
	memset(sala->nomes, 0, sizeof(sala->nomes));
	memset(sala->conexoes, -1, sizeof(sala->conexoes));

	return sala;
}


void registrar_nome(Chroom* sala){
	char nome[] = "Como gostaria de ser conhecido? Digite um ID de ate 4 letras: ";
	send(sala->conexoes[sala->atual].fd, nome, sizeof(nome), 0);
	memset(nome, 0, sizeof(nome));
	int bytes = recv(sala->conexoes[sala->atual].fd, nome, sizeof(nome), 0);
	nome[4] = '\0';
	if(bytes > 0){
		strncpy(sala->nomes+(sala->atual*4), nome, 4);
		printf("%s entrou.\n", nome);
	} else if(bytes == 0){
		printf("Desconhecido desconectou.\n");
	}
}


void* aceitar_chatter(void* ptr){
	Chroom* sala = (Chroom*)ptr;
	struct pollfd listener[1];	
	listener[0].fd = sala->fd;
	listener[0].events = POLLIN;

	while(!exit_flag){
		if(sala->atual == sala->capacidade){
			sala->capacidade *= 2;
			sala->conexoes = realloc(sala->conexoes, sizeof(*(sala->conexoes)) * sala->capacidade);
			sala->nomes = realloc(sala->nomes, sizeof(*(sala->nomes)) * sala->capacidade * 4);
			printf("Servidor expandindo para %d.\n", sala->capacidade);
		}	
		if(poll(listener, 1, 2500)){
			sala->conexoes[sala->atual].fd = aceitar_conexao(sala->fd);
			sala->conexoes[sala->atual].events = POLLIN;
			registrar_nome(sala);
			sala->atual++;
		}
	}
	printf("Exiting accept thread.\n");
}


void ecoar_mensagem(Chroom* sala, int origem){
	char msg[215] = { 0 };
	strncpy(msg, sala->nomes + (origem*4), 4);
	strcat(msg, ": ");
	strncat(msg, sala->msg_buff, sizeof(sala->msg_buff));
	if(sala->msg_buff[strlen(sala->msg_buff)-1] != '\n'){
		strcat(msg, "\n");
	}
	for(int i = 0; i < sala->atual; i++){
		if(sala->conexoes[i].fd != sala->conexoes[origem].fd){
			send(sala->conexoes[i].fd, msg, sizeof(msg), 0);
		}
	}
	memset(sala->msg_buff, 0, sizeof(sala->msg_buff));
}


void* ouvir_chatters(void* ptr){
	Chroom* sala = (Chroom*)ptr;
	char quitter[4];
	int bytes;
	while(!exit_flag){	
		poll(sala->conexoes, sala->atual, 0);
		for(int i = 0; i < sala->atual; i++){
			if(!(sala->conexoes[i].revents & POLLIN)){
				continue;
			}
			bytes = recv(sala->conexoes[i].fd, (char*)sala->msg_buff, sizeof(sala->msg_buff), 0); 
			if(bytes > 0){
				ecoar_mensagem(sala, i);
			} else if(bytes == 0){
				close(sala->conexoes[i].fd);
				sala->atual--;
				memcpy(&sala->conexoes[i], &sala->conexoes[sala->atual], sizeof(*(sala->conexoes)));
				strncpy(quitter, sala->nomes + (i*4), sizeof(quitter));
				printf("%s saiu.\n", quitter);
				memcpy(sala->nomes+(i*4), sala->nomes+(sala->atual*4), 4);
			}
		}
	}
	printf("Exiting listener thread.\n");
}


void fechar_sala(Chroom* sala){
	if(sala){
		if(sala->fd >= 0){
			close(sala->fd);
		}
		if(sala->conexoes){
			for(int i = 0; i < sala->atual; i++){
				if(sala->conexoes[i].fd >= 0){
					close(sala->conexoes[i].fd);
				}
			}
			free(sala->conexoes);
		}
		if(sala->nomes){
			free(sala->nomes);
		}
		free(sala);
	}
}
