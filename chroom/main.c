#include "chroom.h"
#include "../conexao.h"
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>


void sigint_handler(int signum){
	exit_flag = 1;
}


int main(int argc, char* argv[]){
	signal(SIGINT, sigint_handler);	
		
	if(argc < 2){
		printf("missing arguments.\n");
		return 1;
	}
	Chroom* sala = abrir_sala(argv[1]);
	if(sala == NULL){
		printf("Incapaz de abrir sala.\nEncerrando.\n");
		return 1;
	}
	pthread_t aceitar, ouvir;

	pthread_create(&aceitar, NULL, aceitar_chatter, sala);
	pthread_create(&ouvir, NULL, ouvir_chatters, sala);

	pthread_join(aceitar, NULL);
	pthread_join(ouvir, NULL);
	fechar_sala(sala);

	return 0;
}
