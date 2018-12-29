CC=c++
CFLAGS=-Wall -Wextra -I/opt/local/include -I../../parser -ggdb3 -std=c++11

run.exe: parser_printer.cpp lex.yy.o
	$(CC) $(CFLAGS) -o run.exe parser_printer.cpp lex.yy.o

debug: parser_printer.cpp lex.yy.o
	$(CC) $(CFLAGS) -o debug parser_printer.cpp lex.yy.o -DDEBUG=

lex.yy.o: lex.yy.c
	cc -c lex.yy.c

lex.yy.c: parser_printer.l
	flex parser_printer.l

clean:
	rm *.o *.dot *.png debug run.exe lex.yy.c valgrind-out.txt

graphviz:
	dot -Tpng tree.dot > tree.png

valgrind:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out.txt ./run.exe
