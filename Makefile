chat: chroom.o conexao.o chroom/main.c
	gcc chroom/main.c conexao.o chroom.o -o chat -lpthread

client: chatter.o conexao.o chatter/main.c
	gcc chatter/main.c chatter.o conexao.o -o client -lpthread

chroom.o: chroom/chroom.c chroom/chroom.h
	gcc -c chroom/chroom.c

chatter.o: chatter/chatter.c chatter/chatter.h
	gcc -c chatter/chatter.c

conexao.o: conexao.c conexao.h
	gcc -c conexao.c

clean:
	rm -rf *.o chat client
