#include "chatter.h"
#include "../conexao.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

#define xmalloc(...) NULL

int exit_flag = 0;


Chatter* criar_chatter(){
	Chatter* chatter = malloc(sizeof(*chatter));
	chatter->in_size = 100;
	chatter->out_size = 100;
	chatter->buff_in = malloc(sizeof(*chatter->buff_in) * chatter->in_size);
	chatter->buff_out = malloc(sizeof(*chatter->buff_out) * chatter->out_size);
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


void ler_input(Chatter* chatter){
	size_t str_len = 200;
	char* str = malloc(sizeof(*str) * str_len);
	size_t desired_size = 0;
	size_t chunk_size;
	char* save_me = NULL;
	int currently_at = 0;
	if(chatter->buff_in == NULL){
		chatter->in_size = str_len;
		chatter->buff_in = malloc(sizeof(*(chatter->buff_in))*(chatter->in_size));
	}

	while(fgets(str, str_len, stdin)){
		desired_size += str_len;
		if(desired_size > chatter->in_size){
			save_me = realloc(chatter->buff_in, desired_size); 
			if(save_me == NULL){
				printf("NOOOOOOO.\n");
				break;
			}
			chatter->buff_in = save_me;
			chatter->in_size = desired_size;
		}
		memcpy((chatter->buff_in + currently_at), str, str_len); 
		currently_at = strnlen(chatter->buff_in, chatter->in_size);
		memset(str, 0, str_len);
	}
	if(str){
		free(str);
	}
}


void* enviar_mensagem(void* ptr){
	Chatter* chatter = (Chatter*)ptr;
	int size;
	while(!exit_flag){
		if(chatter->fd == -1){
			break;
		}
		ler_input(chatter);
		if(strnlen(chatter->buff_in, chatter->in_size) > 0){
			size = htonl(strnlen(chatter->buff_in, chatter->in_size));
			send(chatter->fd, &size, sizeof(size), 0);
			send(chatter->fd, chatter->buff_in, strnlen(chatter->buff_in, chatter->in_size), 0);		
			memset(chatter->buff_in, 0, chatter->in_size);
		}
	}
}


void estender_buffer(Chatter* chatter, int size){
	char* extended = NULL;
	if(chatter->out_size < size){
		extended = realloc(chatter->buff_out, size);
		if(extended == NULL){
			printf("NOOOOOO.\n");
			return;
		}
		chatter->buff_out = extended;
		chatter->out_size = size;
	}
	return;
}


int ouvir_mensagem(Chatter* chatter){
	int read_bytes = 0;
	int size_network, size;
	int chunk_size = 0;
	char msg_chunk[200];

	chunk_size = recv(chatter->fd, &size_network, sizeof(size_network), 0);
	if(chunk_size == 0){
		/* Disconnect */
		printf("Connection closed by foreign host.\n");
		close(chatter->fd);
		chatter->fd = -1;
		return read_bytes;
	} else if(chunk_size < 0){
		/* Error */
		return read_bytes;
	}
	size = ntohl(size_network);
	estender_buffer(chatter, size);	
	while(read_bytes < size){
		chunk_size = recv(chatter->fd, msg_chunk, sizeof(msg_chunk), 0);
		if(chunk_size > 0){
			memcpy((chatter->buff_out + read_bytes), msg_chunk, chunk_size);	
			read_bytes += chunk_size;
		} else if(chunk_size == 0){
			/* Disconnect */
			printf("Connection closed by foreign host.\n");
			close(chatter->fd);
			chatter->fd = -1;
			break;
		} else{
			/* Error */
		}
	}
	return read_bytes;
}


void* ouvir_sala(void* ptr){
	Chatter* chatter = (Chatter*)ptr;
	int bytes;
	while(!exit_flag){
		if(chatter->fd == -1){
			break;
		}
		bytes = ouvir_mensagem(chatter);
		if(bytes > 0){
			printf("%s", chatter->buff_out);	
			memset(chatter->buff_out, 0, chatter->out_size);
		}
	}
}


void apagar_chatter(Chatter* chatter){
	if(chatter){
		if(chatter->fd >= 0){
			close(chatter->fd);
		}
		if(chatter->buff_in){
			free(chatter->buff_in);
		}
		if(chatter->buff_out){
			free(chatter->buff_out);
		}
		free(chatter);
	}
}
