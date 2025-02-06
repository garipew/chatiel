#include "chroom.h"
#include "../conexao.h"
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>


Chroom* sala = NULL;

void sigint_handler(int signum){
	fechar_sala(sala);
	exit(0);
}

int main(int argc, char* argv[]){
		
	if(argc < 2){
		printf("missing arguments.\n");
		return 1;
	}
	sala = abrir_sala(argv[1]);
	pthread_t aceitar, ouvir;

	pthread_create(&aceitar, NULL, aceitar_chatter, sala);
	pthread_create(&ouvir, NULL, ouvir_chatters, sala);

	pthread_join(aceitar, NULL);
	pthread_join(ouvir, NULL);
	fechar_sala(sala);

	return 0;
}
