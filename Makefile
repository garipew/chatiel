chat: chroom.o conexao.o server/main.c
	gcc server/main.c conexao.o chroom.o -o chat -lpthread

chatter: chatter.o conexao.o client/main.c
	gcc client/main.c chatter.o conexao.o -o chatter -lpthread

chroom.o: server/chroom.c server/chroom.h
	gcc -c server/chroom.c

chatter.o: client/chatter.c client/chatter.h
	gcc -c client/chatter.c

conexao.o: conexao.c conexao.h
	gcc -c conexao.c

clean:
	rm -rf *.o chat chatter
