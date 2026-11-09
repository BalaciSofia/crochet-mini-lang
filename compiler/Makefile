CC = gcc
CFLAGS = -Wall -Wextra -g

TOKENS = ../tokens.txt
EXAMPLES = ../examples
OUT = ../out

lexer: lex.yy.c symtab.c symtab.h
	$(CC) $(CFLAGS) -o lexer lex.yy.c symtab.c

lex.yy.c: lexer.l
	flex lexer.l

test: lexer
	mkdir -p $(OUT)
	./lexer $(TOKENS) $(EXAMPLES)/scarf.cml $(OUT)
	./lexer $(TOKENS) $(EXAMPLES)/blanket.cml $(OUT)
	./lexer $(TOKENS) $(EXAMPLES)/lexical-errors.cml $(OUT) || true

clean:
	rm -rf lexer lex.yy.c
