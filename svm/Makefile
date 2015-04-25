CC ?= clang
LD ?= clang

CFLAGS += -Wall -g -std=c99

.SUFFIXES: .o .c .h
.PHONY: all clean

all: svm

.c.o:
	@echo -e "CC  $<"
	@$(CC) -c $^ -o $@ $(CFLAGS)

svm: main.o lexer.o dl_list.o
	@echo -e "LD  $@"
	@$(CC) $^ -o $@ $(LDFLAGS)

clean:
	rm -f *.o
	rm -f *.so
	rm -f svm
