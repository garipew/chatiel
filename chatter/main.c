#include "chatter.h"
#include "../conexao.h"
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>


Chatter* chatter = NULL;

void sigint_handler(int signum){
	apagar_chatter(chatter);
	exit(0);
}


int main(int argc, char* argv[]){
		
	if(argc < 3){
		printf("missing arguments.\n");
		return 1;
	}
	chatter = conectar_sala(argv[1], argv[2]);
	printf("%s conectado\n", chatter->nome);

	pthread_t enviar, ouvir;
	pthread_create(&enviar, NULL, enviar_mensagem, chatter);
	pthread_create(&ouvir, NULL, ouvir_sala, chatter);

	pthread_join(enviar, NULL);
	pthread_join(ouvir, NULL);
	apagar_chatter(chatter);
	return 0;
}
