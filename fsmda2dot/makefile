CC=gcc
CFLAGS=-g -c
INCLUDES=-I ./include/
LEX=flex
LFLAGS=
YACC=bison
YFLAGS=-dv

all: fsmda2dot

#creates the executable file
fsmda2dot : fsmda.tab.o lex.yy.o
	$(CC) -g -o ./bin/fsmda2dot ./obj/*.o


#creates the object files
fsmda.tab.o : parser
	$(CC) $(CFLAGS) $(INCLUDES) ./src/fsmda.tab.c -o ./obj/fsmda.tab.o

lex.yy.o : parser
	$(CC) $(CFLAGS) $(INCLUDES) ./src/lex.yy.c -o ./obj/lex.yy.o

#calls the parser
parser : lexer
	$(YACC) $(YFLAGS) ./src/fsmda.y
	mv ./fsmda.tab.h ./include/fsmda.tab.h
	mv ./fsmda.tab.c ./src/fsmda.tab.c
	mv ./fsmda.output ./obj/fsmda.output

#calls the lexer
lexer :  
	$(LEX) $(LFLAGS) ./src/fsmda.lex
	mv ./lex.yy.c ./src/lex.yy.c


clean :
	rm -rf ./obj/*.o
	rm -rf ./obj/*.output
	rm -rf ./include/*.tab.h
	rm -rf ./bin/fsmda2dot
	rm -rf ./src/*.tab.c
	rm -rf ./src/lex.yy.c
