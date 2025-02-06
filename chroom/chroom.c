#include "chroom.h"
#include "../conexao.h"
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


Chroom* abrir_sala(char* port){
	Chroom* sala = malloc(sizeof(*sala));

	sala->fd = encontrar_conexao(NULL, port);
	for(int i = 0; i < 10; i++){
		sala->conexoes[i].fd = -1;
	}

	return sala;
}


void* aceitar_chatter(void* ptr){
	Chroom* sala = (Chroom*)ptr;
	while(1){
		for(int i = 0; i < 10; i++){
			if(sala->conexoes[i].fd == -1){
				sala->conexoes[i].fd = aceitar_conexao(sala->fd);
				sala->conexoes[i].events = POLLIN;
			}
		}	
	}
}


void ecoar_mensagem(Chroom* sala, int origem){
	for(int i = 0; i < 10; i++){
		if(sala->conexoes[i].fd != origem){
			send(sala->conexoes[i].fd, sala->msg_buff, sizeof(sala->msg_buff), 0);
		}
	}
}


void* ouvir_chatters(void* ptr){
	Chroom* sala = (Chroom*)ptr;
	while(1){	
		poll(sala->conexoes, 10, 2500);
		for(int i = 0; i < 10; i++){
			if(!(sala->conexoes[i].revents & POLLIN)){
				continue;
			}
			if(recv(sala->conexoes[i].fd, (char*)sala->msg_buff, sizeof(sala->msg_buff), 0)){
				ecoar_mensagem(sala, sala->conexoes[i].fd);
			}
		}
	}
}


void fechar_sala(Chroom* sala){
	close(sala->fd);
	free(sala);
}
