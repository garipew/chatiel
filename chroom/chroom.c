#include "chroom.h"
#include "../conexao.h"
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


Chroom* abrir_sala(char* port){
	Chroom* sala = malloc(sizeof(*sala));

	sala->fd = encontrar_conexao(NULL, port);
	sala->capacidade = 5;
	sala->atual = 0;
	sala->conexoes = malloc(sizeof(*(sala->conexoes)) * sala->capacidade);
	memset(sala->conexoes, -1, sizeof(sala->conexoes));

	return sala;
}


void* aceitar_chatter(void* ptr){
	Chroom* sala = (Chroom*)ptr;
	while(1){
		if(sala->atual == sala->capacidade){
			sala->capacidade *= 2;
			sala->conexoes = realloc(sala->conexoes, sizeof(*(sala->conexoes)) * sala->capacidade);
			printf("Expanding server size to %d.\n", sala->capacidade);
		}	
		sala->conexoes[sala->atual].fd = aceitar_conexao(sala->fd);
		sala->conexoes[sala->atual].events = POLLIN;
		sala->atual++;
	}
}


void ecoar_mensagem(Chroom* sala, int origem){
	for(int i = 0; i < sala->capacidade; i++){
		if(sala->conexoes[i].fd != origem){
			send(sala->conexoes[i].fd, sala->msg_buff, sizeof(sala->msg_buff), 0);
		}
	}
}


void* ouvir_chatters(void* ptr){
	Chroom* sala = (Chroom*)ptr;
	while(1){	
		poll(sala->conexoes, sala->capacidade, 2500);
		for(int i = 0; i < sala->capacidade; i++){
			if(!(sala->conexoes[i].revents & POLLIN)){
				continue;
			}
			if(recv(sala->conexoes[i].fd, (char*)sala->msg_buff, sizeof(sala->msg_buff), 0)){
				ecoar_mensagem(sala, sala->conexoes[i].fd);
			} else{
				close(sala->conexoes[i].fd);
				sala->atual--;
				memcpy(&sala->conexoes[i], &sala->conexoes[sala->atual], sizeof(*(sala->conexoes)));
				printf("Currently %d online.\n", sala->atual);
			}
		}
	}
}


void fechar_sala(Chroom* sala){
	close(sala->fd);
	for(int i = 0; i < sala->capacidade; i++){
		close(sala->conexoes[i].fd);
	}
	free(sala->conexoes);
	free(sala);
}
