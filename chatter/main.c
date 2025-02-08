#include "chatter.h"
#include "../conexao.h"
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>


void sigint_handler(int signum){
	exit_flag = 1;
}


int main(int argc, char* argv[]){
	signal(SIGINT, sigint_handler);	
		
	if(argc < 3){
		printf("missing arguments.\n");
		return 1;
	}
	fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
	Chatter* chatter = conectar_sala(argv[1], argv[2]);
	if(chatter == NULL){
		printf("Incapaz de criar chatter.\nEncerrando.\n");
		return 1;
	}
	printf("Conectado ao servidor\n");

	pthread_t enviar, ouvir;
	pthread_create(&enviar, NULL, enviar_mensagem, chatter);
	pthread_create(&ouvir, NULL, ouvir_sala, chatter);

	pthread_join(enviar, NULL);
	pthread_join(ouvir, NULL);
	apagar_chatter(chatter);
	return 0;
}
