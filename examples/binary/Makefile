CC=c++
CFLAGS=-Wall -Wextra -I/opt/local/include -I../../parser -std=c++11 -ggdb3

run.exe: binary.cpp lex.yy.o
	$(CC) $(CFLAGS) -o run.exe binary.cpp lex.yy.o

debug: binary.cpp lex.yy.o
	$(CC) $(CFLAGS) -o debug binary.cpp lex.yy.o -DDEBUG=

lex.yy.o: lex.yy.c
	cc -c lex.yy.c

lex.yy.c: binary.l
	flex binary.l

clean:
	rm *.o *.dot *.png debug run.exe lex.yy.c valgrind-out.txt

valgrind:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out.txt ./run.exe
