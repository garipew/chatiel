#include "chroom.h"
#include "../conexao.h"
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#define xmalloc(...) NULL
#define xrealloc(...) NULL

int exit_flag = 0;


Chroom* abrir_sala(char* port){
	Chroom* sala = malloc(sizeof(*sala));
	if(sala == NULL){
		goto sala;
	}

	sala->fd = encontrar_conexao(NULL, port);
	sala->capacidade = 5;
	sala->atual = 0;
	sala->max_size = 0;
	sala->msg_size = 100;
	sala->msg_buff = malloc(sizeof(*(sala->msg_buff)) * sala->msg_size);
	if(sala->msg_buff == NULL){
		goto msg;
	}
	sala->conexoes = malloc(sizeof(*(sala->conexoes)) * sala->capacidade);
	if(sala->conexoes == NULL){
		goto conexoes;
	}
	memset(sala->conexoes, -1, sizeof(sala->conexoes));
	sala->nomes = malloc(sizeof(*(sala->nomes)) * sala->capacidade * 4);
	if(sala->nomes == NULL){
		goto nomes;
	}
	memset(sala->nomes, 0, sizeof(sala->nomes));
	goto sala;

	nomes:
	free(sala->conexoes);
	sala->conexoes = NULL;
	conexoes:
	free(sala->msg_buff);
	sala->msg_buff = NULL;
	msg:
	free(sala);
	sala = NULL;
	sala:
	return sala;
}


void enviar_mensagem(int fd, char* str, size_t str_len){
	int size = htonl(strnlen(str, str_len));
	send(fd, &size, sizeof(size), 0);
	send(fd, str, strnlen(str, str_len), 0);	
}


void registrar_nome(Chroom* sala){
	char nome[] = "Como gostaria de ser conhecido? Digite um ID de ate 4 letras: ";
	enviar_mensagem(sala->conexoes[sala->atual].fd, nome, sizeof(nome));
	int bytes = ouvir_mensagem(sala, sala->atual);
	if(bytes > 0){
		strncpy(sala->nomes+(sala->atual*4), sala->msg_buff, 4);
		memset(sala->msg_buff + 4, 0, sala->msg_size - 4);
		strcat(sala->msg_buff, " entrou.\n"); 
		ecoar_mensagem(sala, strnlen(sala->msg_buff, sala->msg_size), -1);
	}
}


void* aceitar_chatter(void* ptr){
	Chroom* sala = (Chroom*)ptr;
	struct pollfd listener[1];	
	listener[0].fd = sala->fd;
	listener[0].events = POLLIN;
	char *name_buff;
	struct pollfd* con_buff;	

	while(!exit_flag){
		if((sala->atual == sala->capacidade) && !sala->max_size){
			sala->capacidade *= 2;
			con_buff = realloc(sala->conexoes, sizeof(*(sala->conexoes)) * sala->capacidade);
			if(con_buff == NULL){
				sala->max_size = 1;
				sala->capacidade /= 2;
				printf("Servidor não consegue expandir... Dx\n%d/%d\n", sala->atual, sala->capacidade);
				continue;
			}
			name_buff = realloc(sala->nomes, sizeof(*(sala->nomes)) * sala->capacidade * 4);
			if(name_buff == NULL){
				free(con_buff);
				sala->max_size = 1;
				sala->capacidade /= 2;
				printf("Servidor não consegue expandir... Dx\n%d/%d\n", sala->atual, sala->capacidade);
				continue;
			}
			
			sala->nomes = name_buff;
			sala->conexoes = con_buff;
			printf("Servidor expandindo para %d.\n", sala->capacidade);
		}	
		if((sala->atual < sala->capacidade) && poll(listener, 1, 2500)){
			sala->conexoes[sala->atual].fd = aceitar_conexao(sala->fd);
			sala->conexoes[sala->atual].events = POLLIN;
			registrar_nome(sala);
			sala->atual++;
		}
	}
	printf("Exiting accept thread.\n");
}


/* void ecoar_mensagem(Chroom* sala, int bytes, int origem)
 * Envia bytes da string em msg_buff de sala para todos os clients,
 * com excecao do client de origem. 
 * Caso origem nao exista, envia para todos.
*/
void ecoar_mensagem(Chroom* sala, int bytes, int origem){
	int msg_size = bytes + 7;
	char* msg = malloc(sizeof(*msg) * (msg_size));
	int total_bytes;
	if(msg == NULL){
		printf("-_-\n");
		return;
	}
	memset(msg, 0, msg_size);
	if(origem >= 0 && origem < sala->atual){
		strncpy(msg, sala->nomes + (origem*4), 4);
		strcat(msg, ": ");
	} else{
		printf("%s", sala->msg_buff);
	} 
	strncat(msg, sala->msg_buff, sala->msg_size);
	total_bytes = strnlen(msg, msg_size);
	total_bytes = htonl(total_bytes);
	for(int i = 0; i < sala->atual; i++){
		if(origem < 0 || origem >= sala->atual || sala->conexoes[i].fd != sala->conexoes[origem].fd){
			send(sala->conexoes[i].fd, &total_bytes, sizeof(total_bytes), 0);
			send(sala->conexoes[i].fd, msg, strnlen(msg, msg_size), 0);
		}
	}
	memset(sala->msg_buff, 0, sala->msg_size);
	if(msg){
		free(msg);
	}
}


void remover_chatter(Chroom* sala, int i){
	if(sala->conexoes[i].fd >= 0){
		close(sala->conexoes[i].fd);
		sala->atual--;
		memcpy(&sala->conexoes[i], &sala->conexoes[sala->atual], sizeof(*(sala->conexoes)));
		strncpy(sala->msg_buff, sala->nomes + (i*4), 4);
		strcat(sala->msg_buff, " saiu.\n");
		memcpy(sala->nomes+(i*4), sala->nomes+(sala->atual*4), 4);
		ecoar_mensagem(sala, strnlen(sala->msg_buff, sala->msg_size), -1);
	}
}


void estender_buffer(Chroom* sala, size_t size){
	char* extended;
	if(sala->msg_size < size){
		extended = realloc(sala->msg_buff, size);
		if(extended == NULL){
			printf("NOOOOOO.\n");
			return;
		}
		sala->msg_buff = extended;
		sala->msg_size = size;
	}
}

int ouvir_mensagem(Chroom* sala, int chatter){
	int read_bytes = 0;
	int size;
	int chunk_size = 0;
	char msg_chunk[200];

	chunk_size = recv(sala->conexoes[chatter].fd, &size, sizeof(size), 0);
	if(chunk_size == 0){
		/* Disconnect */
		remover_chatter(sala, chatter);
		return read_bytes;
	} else if(chunk_size < 0){
		/* Error */
		return read_bytes;
	}
	size = ntohl(size);
	estender_buffer(sala, size);	
	memset(sala->msg_buff, 0, sala->msg_size);
	while(read_bytes < size){
		chunk_size = recv(sala->conexoes[chatter].fd, msg_chunk, sizeof(msg_chunk), 0);
		if(chunk_size == 0){
			/* Disconnect */
			remover_chatter(sala, chatter);
			break;
		} else if(chunk_size < 0){
			/* Error */
			break;
		}
		memcpy((sala->msg_buff + read_bytes), msg_chunk, chunk_size);	
		read_bytes += chunk_size;
	}
	return read_bytes;
}


void* ouvir_chatters(void* ptr){
	Chroom* sala = (Chroom*)ptr;
	int bytes = 0;
	while(!exit_flag){	
		if(sala->atual == 0){
			continue;
		}
		poll(sala->conexoes, sala->atual, 1000);
		for(int i = 0; i < sala->atual; i++){
			if(!(sala->conexoes[i].revents & POLLIN)){
				continue;
			}
			bytes = ouvir_mensagem(sala, i); 
			if(bytes > 0){
				ecoar_mensagem(sala, bytes, i);
			} 
		}
	}
	printf("Exiting listener thread.\n");
}


void fechar_sala(Chroom* sala){
	if(sala){
		strncpy(sala->msg_buff, "See you space cowboy...\n", 25);
		ecoar_mensagem(sala, strnlen(sala->msg_buff, sala->msg_size), -1);
		if(sala->fd >= 0){
			close(sala->fd);
		}
		if(sala->conexoes){
			while(sala->atual > 0){
				remover_chatter(sala, 0);
			}
			free(sala->conexoes);
		}
		if(sala->nomes){
			free(sala->nomes);
		}
		if(sala->msg_buff){
			free(sala->msg_buff);
		}
		free(sala);
	}
}
