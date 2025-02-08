#include "chatter.h"
#include "../conexao.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#define xmalloc(...) NULL

int exit_flag = 0;


Chatter* criar_chatter(){
	Chatter* chatter = malloc(sizeof(*chatter));
	return chatter;
}

Chatter* conectar_sala(char* ip, char* port){
	Chatter* chatter = criar_chatter();
	if(chatter == NULL){
		return chatter;
	}
	chatter->fd = encontrar_conexao(ip, port);

	if(chatter->fd == -1){
		free(chatter);
		return NULL;
	}
	fcntl(chatter->fd, F_SETFL, O_NONBLOCK);

	return chatter;
}


void* enviar_mensagem(void* ptr){
	Chatter* chatter = (Chatter*)ptr;
	while(!exit_flag){
		if(chatter->fd == -1){
			break;
		}
		while(fgets(chatter->msg_buff, sizeof(chatter->msg_buff), stdin)){ 
			send(chatter->fd, chatter->msg_buff, strnlen(chatter->msg_buff, sizeof(chatter->msg_buff)), 0);		
		}
	}
}


void* ouvir_sala(void* ptr){
	Chatter* chatter = (Chatter*)ptr;
	int bytes;
	while(!exit_flag){
		bytes = recv(chatter->fd, (char*)chatter->msg_buff, sizeof(chatter->msg_buff), 0);
		if(bytes > 0){
			printf("%s", chatter->msg_buff);	
			memset(chatter->msg_buff, 0, sizeof(chatter->msg_buff));
		} else if(bytes == 0) {
			printf("Server said: See you space cowboy!\n");
			close(chatter->fd);
			chatter->fd = -1;
			break;
		}	
	}
}


void apagar_chatter(Chatter* chatter){
	if(chatter){
		if(chatter->fd >= 0){
			close(chatter->fd);
		}
		free(chatter);
	}
}
